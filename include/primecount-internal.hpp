///
/// @file   primecount-internal.hpp
/// @brief  primecount internal function definitions.
///
/// Copyright (C) 2016 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMECOUNT_INTERNAL_HPP
#define PRIMECOUNT_INTERNAL_HPP

#include <int128.hpp>
#include <aligned_vector.hpp>
#include <pmath.hpp>
#include <print.hpp>

#include <stdint.h>
#include <algorithm>
#include <string>
#include <vector>

namespace primecount {

/// Silence unused parameter compiler warning
template<class T>
void unused_param(const T&)
{ }

std::string pi(const std::string& x, int threads);

int64_t pi(int64_t x, int threads);

#ifdef HAVE_INT128_T

int128_t pi(int128_t x);

int128_t pi(int128_t x, int threads);

#endif

int64_t pi_deleglise_rivat(int64_t x, int threads);

#ifdef HAVE_INT128_T

int128_t pi_deleglise_rivat(int128_t x);

int128_t pi_deleglise_rivat(int128_t x, int threads);

#endif

int64_t pi_deleglise_rivat1(int64_t x);

int64_t pi_deleglise_rivat2(int64_t x);

int64_t pi_deleglise_rivat_parallel1(int64_t x, int threads);

int64_t pi_deleglise_rivat_parallel2(int64_t x, int threads);

#ifdef HAVE_INT128_T

int128_t pi_deleglise_rivat_parallel3(int128_t x, int threads);

#endif

int64_t pi_legendre(int64_t x, int threads);

int64_t pi_lehmer(int64_t x, int threads);

int64_t pi_lehmer2(int64_t x, int threads);

int64_t pi_lmo(int64_t x, int threads);

int64_t pi_lmo1(int64_t x);

int64_t pi_lmo2(int64_t x);

int64_t pi_lmo3(int64_t x);

int64_t pi_lmo4(int64_t x);

int64_t pi_lmo5(int64_t x);

int64_t pi_lmo_parallel1(int64_t x, int threads);

int64_t pi_lmo_parallel2(int64_t x, int threads);

int64_t pi_lmo_parallel3(int64_t x, int threads);

int64_t pi_meissel(int64_t x, int threads);

int64_t pi_primesieve(int64_t x, int threads);

int64_t phi(int64_t x, int64_t a, int threads);

int64_t Li(int64_t);

int64_t Li_inverse(int64_t);

#ifdef HAVE_INT128_T

int128_t Li(int128_t);

int128_t Li_inverse(int128_t);

#endif

int64_t nth_prime(int64_t n, int threads);

int64_t P2(int64_t x, int64_t y, int threads);

int64_t P3(int64_t x, int64_t a, int threads);

#ifdef HAVE_INT128_T

int128_t P2(int128_t x, int64_t y, int threads);

#endif

void set_status_precision(int precision);

int get_status_precision(maxint_t x);

void set_alpha(double alpha);

double get_alpha();

double get_alpha(maxint_t x, int64_t y);

double get_alpha_lmo(maxint_t x);

double get_alpha_deleglise_rivat(maxint_t x);

double get_wtime();

int ideal_num_threads(int threads, int64_t sieve_limit, int64_t thread_threshold = 100000);

maxint_t to_maxint(const std::string& expr);

template <typename T>
T get_percent(T low, T limit)
{
  T percent = (T) (100.0 * low / std::max<T>(1, limit));
  return in_between(0, percent, 100);
}

template <typename T>
T S2_approx(T x, int64_t pi_y, T P2, T S1)
{
  T pix = Li(x);
  T S2 = pix - S1 - pi_y + 1 + P2;
  return S2;
}

bool test();

#ifdef HAVE_MPI

class PiTable;

std::vector<int64_t> phi_vector(int64_t x, int64_t a, std::vector<int64_t>& primes, PiTable& pi, int threads);

bool is_mpi_master_proc();
int mpi_num_procs();
int mpi_proc_id();
int mpi_master_proc_id();

int64_t P2_mpi(int64_t x, int64_t y, int threads);

#ifdef HAVE_INT128_T

int128_t P2_mpi(int128_t x, int64_t y, int threads);

#endif /* HAVE_INT128_T */

#endif /* HAVE_MPI */

} // namespace

#endif
