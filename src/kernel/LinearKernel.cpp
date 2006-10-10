/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 1999-2006 Soeren Sonnenburg
 * Copyright (C) 1999-2006 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#include "lib/config.h"

#ifdef HAVE_LAPACK
extern "C" {
#include <cblas.h>
}
#endif

#include "lib/common.h"
#include "lib/io.h"
#include "kernel/LinearKernel.h"
#include "features/Features.h"
#include "features/RealFeatures.h"

CLinearKernel::CLinearKernel(LONG size, bool do_rescale_, DREAL scale_)
  : CSimpleKernel<DREAL>(size),scale(scale_),do_rescale(do_rescale_), normal(NULL)
{
	properties |= KP_LINADD;
}

CLinearKernel::~CLinearKernel() 
{
	cleanup();
}
  
bool CLinearKernel::init(CFeatures* l, CFeatures* r, bool do_init)
{
	CSimpleKernel<DREAL>::init(l, r, do_init); 

	if (do_init)
		init_rescale() ;

	CIO::message(M_INFO, "rescaling kernel by %g (num:%d)\n",scale, CMath::min(l->get_num_vectors(), r->get_num_vectors()));

	return true;
}

void CLinearKernel::init_rescale()
{
	if (!do_rescale)
		return ;
	double sum=0;
	scale=1.0;
	for (INT i=0; (i<lhs->get_num_vectors() && i<rhs->get_num_vectors()); i++)
			sum+=compute(i, i);

	scale=sum/CMath::min(lhs->get_num_vectors(), rhs->get_num_vectors());
}

void CLinearKernel::cleanup()
{
	delete_optimization();
}

bool CLinearKernel::load_init(FILE* src)
{
	return false;
}

bool CLinearKernel::save_init(FILE* dest)
{
	return false;
}

void CLinearKernel::clear_normal()
{
	int num = ((CRealFeatures*) lhs)->get_num_features();
	if (normal==NULL)
		normal = new DREAL[num] ;
	for (int i=0; i<num; i++)
		normal[i]=0;

	set_is_initialized(true);
}

void CLinearKernel::add_to_normal(INT idx, DREAL weight) 
{
	INT vlen;
	bool vfree;
	double* vec=((CRealFeatures*) lhs)->get_feature_vector(idx, vlen, vfree);

	for (int i=0; i<vlen; i++)
		normal[i]+= weight*vec[i];

	((CRealFeatures*) lhs)->free_feature_vector(vec, idx, vfree);

	set_is_initialized(true);
}
  
DREAL CLinearKernel::compute(INT idx_a, INT idx_b)
{
  INT alen, blen;
  bool afree, bfree;

  double* avec=((CRealFeatures*) lhs)->get_feature_vector(idx_a, alen, afree);
  double* bvec=((CRealFeatures*) rhs)->get_feature_vector(idx_b, blen, bfree);
  
  ASSERT(alen==blen);

  INT ialen=(int) alen;

#ifndef HAVE_LAPACK
  DREAL result=0;
  {
    for (INT i=0; i<ialen; i++)
      result+=avec[i]*bvec[i];
  }
  result/=scale;
#else
  INT skip=1;
  DREAL result = cblas_ddot(ialen, avec, skip, bvec, skip)/scale;
#endif

  ((CRealFeatures*) lhs)->free_feature_vector(avec, idx_a, afree);
  ((CRealFeatures*) rhs)->free_feature_vector(bvec, idx_b, bfree);

  return result;
}

bool CLinearKernel::init_optimization(INT num_suppvec, INT* sv_idx, DREAL* alphas) 
{
	clear_normal();
	for (int i=0; i<num_suppvec; i++)
		add_to_normal(sv_idx[i], alphas[i]);

	set_is_initialized(true);
	return true;
}

bool CLinearKernel::delete_optimization()
{
	delete[] normal;
	normal=NULL;
	set_is_initialized(false);

	return true;
}

DREAL CLinearKernel::compute_optimized(INT idx_b) 
{
	INT blen;
	bool bfree;

	double* bvec=((CRealFeatures*) rhs)->get_feature_vector(idx_b, blen, bfree);

	ASSERT(get_is_initialized());

	int ialen=(int) blen;

#ifndef HAVE_LAPACK
	DREAL result=0;
	{
		for (INT i=0; i<ialen; i++)
			result+=normal[i]*bvec[i];
	}
	result/=scale;
#else
	INT skip=1;
	DREAL result = cblas_ddot(ialen, normal, skip, bvec, skip)/scale;
#endif

	((CRealFeatures*) rhs)->free_feature_vector(bvec, idx_b, bfree);

	return result;
}
