///
/// @file  pi_lmo_parallel1.cpp
/// @brief Parallel implementation of the Lagarias-Miller-Odlyzko
///        prime counting algorithm using OpenMP. This implementation
///        is based on pi_lmo4(x).
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#include <primecount-internal.hpp>
#include <aligned_vector.hpp>
#include <generate.hpp>
#include <pmath.hpp>
#include <PhiTiny.hpp>
#include <S1.hpp>
#include <tos_counters.hpp>

#include <stdint.h>
#include <algorithm>
#include <vector>

#ifdef _OPENMP
  #include <omp.h>
#endif

using namespace std;
using namespace primecount;

namespace {

/// For each prime calculate its first multiple >= low
vector<int64_t> generate_next_multiples(int64_t low, int64_t size, vector<int32_t>& primes)
{
  vector<int64_t> next;
  next.reserve(size);
  next.push_back(0);

  for (int64_t b = 1; b < size; b++)
  {
    int64_t prime = primes[b];
    int64_t next_multiple = ceil_div(low, prime) * prime;
    next.push_back(next_multiple);
  }

  return next;
}

template <typename T1, typename T2>
void cross_off(int64_t prime,
               int64_t low,
               int64_t high,
               int64_t& next_multiple,
               T1& sieve,
               T2& counters)
{
  int64_t segment_size = sieve.size();
  int64_t m = next_multiple + prime * (~next_multiple & 1);

  for (; m < high; m += prime * 2)
  {
    if (sieve[m - low])
    {
      sieve[m - low] = 0;
      cnt_update(counters, m - low, segment_size);
    }
  }

  next_multiple = m;
}

/// Compute the S2 contribution for the interval
/// [low_thread, low_thread + segments * segment_size[.
/// The missing special leaf contributions for the interval
/// [1, low_thread[ are later reconstructed and added in
/// the calling (parent) S2 function.
///
int64_t S2_thread(int64_t x,
                  int64_t y,
                  int64_t c,
                  int64_t segment_size,
                  int64_t segments_per_thread,
                  int64_t thread_num,
                  int64_t limit,
                  vector<int32_t>& pi,
                  vector<int32_t>& primes,
                  vector<int32_t>& lpf,
                  vector<int32_t>& mu,
                  vector<int64_t>& mu_sum,
                  vector<int64_t>& phi)
{
  int64_t low = 1 + segment_size * segments_per_thread * thread_num;
  limit = min(low + segment_size * segments_per_thread, limit);
  int64_t size = pi[min(isqrt(x / low), y)] + 1;
  int64_t pi_y = pi[y];

  if (c >= size - 1)
    return 0;

  int64_t S2_thread = 0;
  vector<char> sieve(segment_size);
  vector<int32_t> counters(segment_size);
  vector<int64_t> next = generate_next_multiples(low, size, primes);
  phi.resize(size, 0);
  mu_sum.resize(size, 0);

  // Process the segments assigned to the current thread
  for (; low < limit; low += segment_size)
  {
    fill(sieve.begin(), sieve.end(), 1);

    // Current segment = interval [low, high[
    int64_t high = min(low + segment_size, limit);
    int64_t b = 1;

    // phi(y, b) nodes with b <= c do not contribute to S2, so we
    // simply sieve out the multiples of the first c primes
    for (; b <= c; b++)
    {
      int64_t k = next[b];
      for (int64_t prime = primes[b]; k < high; k += prime)
        sieve[k - low] = 0;
      next[b] = k;
    }

    // Initialize special tree data structure from sieve
    cnt_finit(sieve, counters, segment_size);

    // For c + 1 <= b < pi_sqrty
    // Find all special leaves: n = primes[b] * m
    // which satisfy:  mu[m] != 0 && primes[b] < lpf[m], low <= (x / n) < high
    for (; b < min(pi_y, size); b++)
    {
      int64_t prime = primes[b];
      int64_t min_m = max(x / (prime * high), y / prime);
      int64_t max_m = min(x / (prime * low), y);

      if (prime >= max_m)
        break;

      for (int64_t m = max_m; m > min_m; m--)
      {
        if (mu[m] != 0 && prime < lpf[m])
        {
          int64_t n = prime * m;
          int64_t count = cnt_query(counters, (x / n) - low);
          int64_t phi_xn = phi[b] + count;
          S2_thread -= mu[m] * phi_xn;
          mu_sum[b] -= mu[m];
        }
      }

      phi[b] += cnt_query(counters, (high - 1) - low);
      cross_off(prime, low, high, next[b], sieve, counters);
    }
  }

  return S2_thread;
}

/// Calculate the contribution of the special leaves.
/// This is a parallel implementation without load balancing.
/// @pre y > 0 && c > 1
///
int64_t S2(int64_t x,
           int64_t y,
           int64_t c,
           vector<int32_t>& primes,
           vector<int32_t>& lpf,
           vector<int32_t>& mu,
           int threads)
{
  int64_t limit = x / y + 1;
  threads = ideal_num_threads(threads, limit);

  int64_t S2_total = 0;
  int64_t segment_size = next_power_of_2(isqrt(limit));
  int64_t segments = ceil_div(limit, segment_size);
  threads = in_between(1, threads, segments);
  int64_t segments_per_thread = ceil_div(segments, threads);

  vector<int32_t> pi = generate_pi(y);
  aligned_vector<vector<int64_t> > phi(threads);
  aligned_vector<vector<int64_t> > mu_sum(threads);

  #pragma omp parallel for num_threads(threads) reduction(+: S2_total)
  for (int i = 0; i < threads; i++)
    S2_total += S2_thread(x, y, c, segment_size, segments_per_thread,
        i, limit, pi, primes, lpf, mu, mu_sum[i], phi[i]);

  // Once all threads have finished reconstruct and add the
  // missing contribution of all special leaves. This must
  // be done in order as each thread (i) requires the sum of
  // the phi values from the previous threads.
  //
  for (int i = 1; i < threads; i++)
  {
    for (size_t j = 0; j < phi[i].size(); j++)
    {
      S2_total += phi[i - 1][j] * mu_sum[i][j];
      phi[i][j] += phi[i - 1][j];
    }
  }

  return S2_total;
}

} // namespace

namespace primecount {

/// Calculate the number of primes below x using the
/// Lagarias-Miller-Odlyzko algorithm.
/// Run time: O(x^(2/3)) operations, O(x^(1/3) * (log x)^2) space.
///
int64_t pi_lmo_parallel1(int64_t x, int threads)
{
  if (x < 2)
    return 0;

  double alpha = get_alpha_lmo(x);
  int64_t x13 = iroot<3>(x);
  int64_t y = (int64_t) (x13 * alpha);
  int64_t c = PhiTiny::get_c(y);
  int64_t p2 = P2(x, y, threads);

  vector<int32_t> mu = generate_moebius(y);
  vector<int32_t> lpf = generate_least_prime_factors(y);
  vector<int32_t> primes = generate_primes(y);

  int64_t pi_y = primes.size() - 1;
  int64_t s1 = S1(x, y, c, threads);
  int64_t s2 = S2(x, y, c, primes, lpf, mu, threads);
  int64_t phi = s1 + s2;
  int64_t sum = phi + pi_y - 1 - p2;

  return sum;
}

} // namespace
