/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 1999-2006 Soeren Sonnenburg
 * Written (W) 1999-2006 Gunnar Raetsch
 * Copyright (C) 1999-2006 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#ifndef _WEIGHTEDDEGREECHARKERNELPOLYA_H___
#define _WEIGHTEDDEGREECHARKERNELPOLYA_H___

#include "lib/common.h"
#include "kernel/SimpleKernel.h"

class CWeightedDegreeCharKernelPolyA: public CSimpleKernel<CHAR>
{
 public:
  CWeightedDegreeCharKernelPolyA(LONG size, DREAL* weights, INT degree, INT max_mismatch) ;
  ~CWeightedDegreeCharKernelPolyA() ;
  
  virtual bool init(CFeatures* l, CFeatures* r, bool do_init);
  virtual void cleanup();

  /// load and save kernel init_data
  bool load_init(FILE* src);
  bool save_init(FILE* dest);

  // return what type of kernel we are Linear,Polynomial, Gaussian,...
  virtual EKernelType get_kernel_type() { return K_WEIGHTEDDEGREEPOLYA; }

  // return the name of a kernel
  virtual const CHAR* get_name() { return "WeightedDegreePolyA" ; } ;

  INT get_max_mismatch() { return max_mismatch ; } ;

 protected:

  /// compute kernel function for features a and b
  /// idx_{a,b} denote the index of the feature vectors
  /// in the corresponding feature object
  DREAL compute(INT idx_a, INT idx_b);
  /*    compute_kernel*/

  virtual void remove_lhs() ;
  virtual void remove_rhs() ;
  DREAL compute_with_offset(INT idx_a, INT offset_a, INT idx_b, INT offset_b) ;

 protected:
  DREAL* weights;
  INT degree;
  INT max_mismatch ;

  double* sqrtdiag_lhs;
  double* sqrtdiag_rhs;

  bool initialized ;
  bool *match_vector ;

  INT **lhs_sites ;
  INT *lhs_sites_num ;
  INT **rhs_sites ;
  INT *rhs_sites_num ;
  INT lhs_num, rhs_num ;

  INT down_stream, up_stream ;

  INT* find_site(char* seq, INT len, INT & num) ;
  
};

#endif
