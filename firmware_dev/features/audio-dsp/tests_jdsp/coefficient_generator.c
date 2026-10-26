#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef QR_FACT_H
#define QR_FACT_H
typedef struct
{
	double *tmp1, *tmp2, *tau_data, *vn1_data, *vn2_data, *work_data;
} QR_heap;
void preallocateMemQR(QR_heap *qr, int m, int n);
void freeMemQR(QR_heap *qr);
void QR_pivoting(QR_heap *qr, double *Amat, int m, int n, double *Qmat, double *Rmat, int *perm);
#endif
void xzlarf(int32_t cols1, int32_t rows1, int32_t iv0, double tau, double C_data[], int32_t ic0, int32_t ldc, double work_data[])
{
	int32_t lastv;
	int32_t lastc;
	int32_t i;
	char exitg2;
	int32_t jy;
	int32_t i3;
	int32_t ia;
	int32_t ix;
	int32_t exitg1;
	double c;
	int32_t i4;
	int32_t ijA;
	if (tau != 0.0)
	{
		lastv = cols1;
		i = iv0 + cols1;
		while ((lastv > 0) && (C_data[i - 2] == 0.0))
		{
			lastv--;
			i--;
		}
		lastc = rows1 - 1;
		exitg2 = 0;
		while ((!exitg2) && (lastc + 1 > 0))
		{
			i = ic0 + lastc * ldc;
			ia = i;
			do
			{
				exitg1 = 0;
				if (ia <= (i + lastv) - 1)
				{
					if (C_data[ia - 1] != 0.0)
						exitg1 = 1;
					else
						ia++;
				}
				else
				{
					lastc--;
					exitg1 = 2;
				}
			} while (exitg1 == 0);
			if (exitg1 == 1)
				exitg2 = 1;
		}
	}
	else
	{
		lastv = 0;
		lastc = -1;
	}
	if (lastv > 0)
	{
		if (lastc + 1 != 0)
		{
			if (0 <= lastc)
				memset(&work_data[0], 0, (lastc + 1) * sizeof(double));
			i = 0;
			i3 = ic0 + ldc * lastc;
			for (jy = ic0; ldc < 0 ? jy >= i3 : jy <= i3; jy += ldc)
			{
				ix = iv0;
				c = 0.0;
				i4 = (jy + lastv) - 1;
				for (ia = jy; ia <= i4; ia++)
				{
					c += C_data[ia - 1] * C_data[ix - 1];
					ix++;
				}
				work_data[i] += c;
				i++;
			}
		}
		if (-tau != 0.0)
		{
			i = ic0 - 1;
			jy = 0;
			for (ia = 0; ia <= lastc; ia++)
			{
				if (work_data[jy] != 0.0)
				{
					c = work_data[jy] * -tau;
					ix = iv0;
					i3 = i + 1;
					i4 = lastv + i;
					for (ijA = i3; ijA <= i4; ijA++)
					{
						C_data[ijA - 1] += C_data[ix - 1] * c;
						ix++;
					}
				}
				jy++;
				i += ldc;
			}
		}
	}
}
void xswap(int32_t rows1, double x_data[], int32_t ix0, int32_t iy0)
{
	int32_t ix;
	int32_t iy;
	int32_t k;
	double temp;
	ix = ix0 - 1;
	iy = iy0 - 1;
	for (k = 0; k < rows1; k++)
	{
		temp = x_data[ix];
		x_data[ix] = x_data[iy];
		x_data[iy] = temp;
		ix++;
		iy++;
	}
}
double rt_hypotd(double u0, double u1)
{
	double y;
	double a;
	double b;
	a = fabs(u0);
	b = fabs(u1);
	if (a < b)
	{
		a /= b;
		y = b * sqrt(a * a + 1.0);
	}
	else if (a > b)
	{
		b /= a;
		y = a * sqrt(b * b + 1.0);
	}
	else
		y = a * sqrt(2.0);
	return y;
}
void xgerc(int m, int n, double alpha1, int ix0, const double y_data[], double A_data[], int ia0, int lda)
{
	double temp;
	int i;
	int ijA;
	int ix;
	int j;
	int jA;
	int jy;
	if (alpha1 != 0.0) {
		jA = ia0;
		jy = 0;
		for (j = 0; j < n; j++) {
			if (y_data[jy] != 0.0) {
				temp = y_data[jy] * alpha1;
				ix = ix0;
				i = m + jA;
				for (ijA = jA; ijA < i; ijA++) {
					A_data[ijA - 1] += A_data[ix - 1] * temp;
					ix++;
				}
			}

			jy++;
			jA += lda;
		}
	}
}
double xnrm2(int n, const double x_data[], int ix0)
{
	double absxk;
	double scale;
	double t;
	double y;
	int k;
	int kend;
	y = 0.0;
	if (n >= 1) {
		if (n == 1) {
			y = fabs(x_data[ix0 - 1]);
		}
		else {
			scale = 3.3121686421112381E-170;
			kend = (ix0 + n) - 1;
			for (k = ix0; k <= kend; k++) {
				absxk = fabs(x_data[k - 1]);
				if (absxk > scale) {
					t = scale / absxk;
					y = y * t * t + 1.0;
					scale = absxk;
				}
				else {
					t = absxk / scale;
					y += t * t;
				}
			}

			y = scale * sqrt(y);
		}
	}

	return y;
}
void xorgqr(int n, int k, double A_data[], int lda, const double tau_data[], double *work_data)
{
	double c;
	int b_i;
	int c_i;
	int exitg1;
	int i;
	int i1;
	int ia;
	int iac;
	int iaii;
	int ic0;
	int itau;
	int ix;
	int lastc;
	int lastv;
	char exitg2;
	i = n - 1;
	for (b_i = k; b_i <= i; b_i++) {
		ia = b_i * lda;
		i1 = n - 1;
		memset(&A_data[ia], 0, (i1 + 1) * sizeof(double));
		A_data[ia + b_i] = 1.0;
	}

	itau = k - 1;

	for (c_i = k; c_i >= 1; c_i--) {
		iaii = c_i + (c_i - 1) * lda;
		if (c_i < n) {
			A_data[iaii - 1] = 1.0;
			ic0 = iaii + lda;
			if (tau_data[itau] != 0.0) {
				lastv = (n - c_i) + 1;
				b_i = (iaii + n) - c_i;
				while ((lastv > 0) && (A_data[b_i - 1] == 0.0)) {
					lastv--;
					b_i--;
				}

				lastc = n - c_i;
				exitg2 = 0;
				while ((!exitg2) && (lastc > 0)) {
					b_i = ic0 + (lastc - 1) * lda;
					ia = b_i;
					do {
						exitg1 = 0;
						if (ia <= (b_i + lastv) - 1) {
							if (A_data[ia - 1] != 0.0)
								exitg1 = 1;
							else
								ia++;
						}
						else {
							lastc--;
							exitg1 = 2;
						}
					} while (exitg1 == 0);

					if (exitg1 == 1)
						exitg2 = 1;
				}
			}
			else {
				lastv = 0;
				lastc = 0;
			}

			if (lastv > 0) {
				if (lastc != 0) {
					memset(&work_data[0], 0, lastc * sizeof(double));

					b_i = 0;
					i = ic0 + lda * (lastc - 1);
					for (iac = ic0; lda < 0 ? iac >= i : iac <= i; iac += lda)
					{
						ix = iaii;
						c = 0.0;
						i1 = (iac + lastv) - 1;
						for (ia = iac; ia <= i1; ia++) {
							c += A_data[ia - 1] * A_data[ix - 1];
							ix++;
						}

						work_data[b_i] += c;
						b_i++;
					}
				}

				xgerc(lastv, lastc, -tau_data[itau], iaii, work_data, A_data, ic0, lda);
			}
		}

		if (c_i < n) {
			b_i = iaii + 1;
			i = (iaii + n) - c_i;
			for (ix = b_i; ix <= i; ix++)
				A_data[ix - 1] *= -tau_data[itau];
		}

		A_data[iaii - 1] = 1.0 - tau_data[itau];
		for (b_i = 0; b_i <= c_i - 2; b_i++)
			A_data[(iaii - b_i) - 2] = 0.0;

		itau--;
	}
}
void xgeqp3(double A_data[], const int A_size[2], double tau_data[], int perm[], double *vn1_data, double *vn2_data, double *work_data)
{
	double s;
	double smax;
	double temp2;
	int b_i;
	int exitg1;
	int i;
	int ic0;
	int ii;
	int ip1;
	int ix;
	int iy;
	int k;
	int lastv;
	int ma;
	int minmana;
	int minmn;
	int mmi;
	int nmi;
	int pvt;
	char exitg2;

	ma = A_size[1];
	pvt = A_size[1];
	minmn = A_size[0];
	if (pvt < minmn) {
		minmn = pvt;
	}

	for (pvt = 0; pvt < A_size[0]; pvt++) {
		smax = xnrm2(A_size[1], A_data, pvt * ma + 1);
		vn1_data[pvt] = smax;
		vn2_data[pvt] = smax;
	}

	for (i = 0; i < minmn; i++) {
		ip1 = i + 2;
		iy = i * ma;
		ii = iy + i;
		nmi = A_size[0] - i;
		mmi = A_size[1] - i;
		if (nmi < 1) {
			minmana = -1;
		}
		else
		{
			minmana = 0;
			if (nmi > 1) {
				ix = i;
				smax = fabs(vn1_data[i]);
				for (k = 2; k <= nmi; k++) {
					ix++;
					s = fabs(vn1_data[ix]);
					if (s > smax) {
						minmana = k - 1;
						smax = s;
					}
				}
			}
		}

		pvt = i + minmana;
		if (pvt + 1 != i + 1) {
			ix = pvt * ma;
			for (k = 0; k < A_size[1]; k++) {
				smax = A_data[ix];
				A_data[ix] = A_data[iy];
				A_data[iy] = smax;
				ix++;
				iy++;
			}

			minmana = perm[pvt];
			perm[pvt] = perm[i];
			perm[i] = minmana;
			vn1_data[pvt] = vn1_data[i];
			vn2_data[pvt] = vn2_data[i];
		}

		if (i + 1 < A_size[1]) {
			temp2 = A_data[ii];
			minmana = ii + 2;
			tau_data[i] = 0.0;
			if (mmi > 0) {
				smax = xnrm2(mmi - 1, A_data, ii + 2);
				if (smax != 0.0) {
					s = hypot(A_data[ii], smax);
					if (A_data[ii] >= 0.0)
						s = -s;
					if (fabs(s) < 1.0020841800044864E-292)
					{
						pvt = -1;
						b_i = ii + mmi;
						do {
							pvt++;
							for (k = minmana; k <= b_i; k++)
								A_data[k - 1] *= 9.9792015476736E+291;
							s *= 9.9792015476736E+291;
							temp2 *= 9.9792015476736E+291;
						} while (!(fabs(s) >= 1.0020841800044864E-292));

						s = hypot(temp2, xnrm2(mmi - 1, A_data, ii + 2));
						if (temp2 >= 0.0)
							s = -s;
						tau_data[i] = (s - temp2) / s;
						smax = 1.0 / (temp2 - s);
						for (k = minmana; k <= b_i; k++)
							A_data[k - 1] *= smax;
						for (k = 0; k <= pvt; k++)
							s *= 1.0020841800044864E-292;
						temp2 = s;
					}
					else
					{
						tau_data[i] = (s - A_data[ii]) / s;
						smax = 1.0 / (A_data[ii] - s);
						b_i = ii + mmi;
						for (k = minmana; k <= b_i; k++)
							A_data[k - 1] *= smax;
						temp2 = s;
					}
				}
			}

			A_data[ii] = temp2;
		}
		else {
			tau_data[i] = 0.0;
		}

		if (i + 1 < A_size[0]) {
			temp2 = A_data[ii];
			A_data[ii] = 1.0;
			ic0 = (ii + ma) + 1;
			if (tau_data[i] != 0.0) {
				lastv = mmi;
				minmana = (ii + mmi) - 1;
				while ((lastv > 0) && (A_data[minmana] == 0.0)) {
					lastv--;
					minmana--;
				}

				pvt = nmi - 1;
				exitg2 = 0;
				while ((!exitg2) && (pvt > 0))
				{
					minmana = ic0 + (pvt - 1) * ma;
					nmi = minmana;
					do {
						exitg1 = 0;
						if (nmi <= (minmana + lastv) - 1)
						{
							if (A_data[nmi - 1] != 0.0)
								exitg1 = 1;
							else
								nmi++;
						}
						else
						{
							pvt--;
							exitg1 = 2;
						}
					} while (exitg1 == 0);
					if (exitg1 == 1)
						exitg2 = 1;
				}
			}
			else {
				lastv = 0;
				pvt = 0;
			}

			if (lastv > 0) {
				if (pvt != 0) {
					memset(&work_data[0], 0, pvt * sizeof(double));

					iy = 0;
					b_i = ic0 + ma * (pvt - 1);
					for (k = ic0; ma < 0 ? k >= b_i : k <= b_i; k += ma) {
						ix = ii;
						smax = 0.0;
						minmana = (k + lastv) - 1;
						for (nmi = k; nmi <= minmana; nmi++) {
							smax += A_data[nmi - 1] * A_data[ix];
							ix++;
						}

						work_data[iy] += smax;
						iy++;
					}
				}

				xgerc(lastv, pvt, -tau_data[i], ii + 1, work_data, A_data, ic0, ma);
			}

			A_data[ii] = temp2;
		}

		for (pvt = ip1; pvt <= A_size[0]; pvt++) {
			minmana = i + (pvt - 1) * ma;
			smax = vn1_data[pvt - 1];
			if (smax != 0.0) {
				s = fabs(A_data[minmana]) / smax;
				s = 1.0 - s * s;
				if (s < 0.0)
					s = 0.0;

				temp2 = smax / vn2_data[pvt - 1];
				temp2 = s * (temp2 * temp2);
				if (temp2 <= 1.4901161193847656E-8) {
					if (i + 1 < A_size[1]) {
						smax = xnrm2(mmi - 1, A_data, minmana + 2);
						vn1_data[pvt - 1] = smax;
						vn2_data[pvt - 1] = smax;
					}
					else {
						vn1_data[pvt - 1] = 0.0;
						vn2_data[pvt - 1] = 0.0;
					}
				}
				else
					vn1_data[pvt - 1] = smax * sqrt(s);
			}
		}
	}
}
void preallocateMemQR(QR_heap *qr, int m, int n)
{
	qr->tmp1 = (double*)malloc(m * n * sizeof(double));
	qr->tmp2 = (double*)malloc(m * m * sizeof(double));
	unsigned int maxMN = max(m, n);
	if (m > n)
		qr->tau_data = (double*)malloc(m * sizeof(double));
	else
		qr->tau_data = (double*)malloc(min(m, n) * sizeof(double));
	qr->vn1_data = (double*)malloc(maxMN * sizeof(double));
	qr->vn2_data = (double*)malloc(maxMN * sizeof(double));
	qr->work_data = (double*)malloc(maxMN * sizeof(double));
}
void freeMemQR(QR_heap *qr)
{
	free(qr->tmp1);
	free(qr->tmp2);
	free(qr->tau_data);
	free(qr->vn1_data);
	free(qr->vn2_data);
	free(qr->work_data);
}
void QR_pivoting(QR_heap *qr, double *Amat, int m, int n, double *Qmat, double *Rmat, int *perm)
{
	unsigned int maxMN = max(m, n);
	if (m > n)
		memset(&qr->tau_data[0], 0, m * sizeof(double));
	else
		memset(&qr->tau_data[0], 0, min(m, n) * sizeof(double));
	memset(&qr->work_data[0], 0, maxMN * sizeof(double));
	memset(&qr->vn1_data[0], 0, maxMN * sizeof(double));
	memset(&qr->vn2_data[0], 0, maxMN * sizeof(double));
	int b_A_size[2];
	int i, j;

	for (i = 0; i < n; i++)
		perm[i] = i;
	if (m > n)
	{
		for (i = 0; i < n; i++)
			for (j = 0; j < m; j++)
				qr->tmp2[j + m * i] = Amat[i + n * j];

		for (i = n + 1; i <= m; i++)
			for (j = 0; j < m; j++)
				qr->tmp2[j + m * (i - 1)] = 0.0;
		b_A_size[1] = m;
		b_A_size[0] = m;
		xgeqp3(qr->tmp2, b_A_size, qr->tau_data, perm, qr->vn1_data, qr->vn2_data, qr->work_data);
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j <= i; j++)
				Rmat[i + n * j] = qr->tmp2[j + m * i];
			for (int j = i + 2; j <= m; j++)
				Rmat[(i + n * (j - 1))] = 0.0;
		}
		memset(&qr->work_data[0], 0, maxMN * sizeof(double));
		xorgqr(m, n, qr->tmp2, m, qr->tau_data, qr->work_data);
	}
	else
	{
		for (i = 0; i < n; i++)
			for (j = 0; j < m; j++)
				qr->tmp1[j + m * i] = Amat[i + n * j];

		b_A_size[1] = m;
		b_A_size[0] = n;
		xgeqp3(qr->tmp1, b_A_size, qr->tau_data, perm, qr->vn1_data, qr->vn2_data, qr->work_data);
		for (i = 0; i < m; i++)
		{
			for (j = 0; j <= i; j++)
				Rmat[i + n * j] = qr->tmp1[j + m * i];
			for (j = i + 2; j <= m; j++)
				Rmat[(i + n * (j - 1))] = 0.0;
		}
		for (i = m + 1; i <= n; i++)
		{
			for (j = 0; j < m; j++)
				Rmat[(n + i + n * (j - 1)) - 1] = qr->tmp1[j + m * (i - 1)];
		}
		memset(&qr->work_data[0], 0, maxMN * sizeof(double));
		xorgqr(m, m, qr->tmp1, m, qr->tau_data, qr->work_data);
		for (i = 0; i < m; i++)
			for (j = 0; j < m; j++)
				qr->tmp2[j + m * i] = qr->tmp1[j + m * i];
	}
	for (i = 0; i < m; i++)
		for (j = 0; j < m; j++)
			Qmat[j + m * i] = qr->tmp2[i + m * j];
}
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
static int8_t sabs8(int8_t i)
{
	int8_t res;
	if (INT8_MIN == i)
		res = INT8_MAX;
	else
		res = i < 0 ? -i : i;
	return res;
}
static void matrix_transpose_matrix_self_product(const double *a, double *product, unsigned int cols1, unsigned int rows1)
{
	unsigned int i, j, k;
	for (i = 0; i < rows1; ++i)
		for (j = 0; j < rows1; ++j)
		{
			double res = 0.0;
			for (k = 0; k < cols1; ++k)
				res += a[k * rows1 + i] * a[k * rows1 + j];
			product[i * rows1 + j] = res;
		}
}
static void matrix_transpose_matrix_self_product_in_out_stride(const double *a, double *product, unsigned int cols1, unsigned int rows1, unsigned int inStride, unsigned int outStride)
{
	unsigned int i, j, k;
	for (i = 0; i < rows1; ++i)
		for (j = 0; j < rows1; ++j)
		{
			double res = 0.0;
			for (k = 0; k < cols1; ++k)
				res += a[k * inStride + i] * a[k * inStride + j];
			product[i * outStride + j] = res;
		}
}
static void matrix_matrix_product(const double *a, const double *b, double *product, int rows1, int cols1, int rows2, int cols2)
{
	int i, j, k;
	for (i = 0; i < rows1; ++i)
		for (j = 0; j < cols2; ++j)
		{
			double res = 0.0;
			for (k = 0; k < cols1; ++k)
				res += a[i * cols1 + k] * b[k * cols2 + j];
			product[i * cols2 + j] = res;
		}
}
static void matrix_transposedMatrix_product(const double *a, const double *b, double *product, unsigned int rows1, unsigned int cols1, unsigned int rows2, int unsigned cols2)
{
	unsigned int i, j, k;
	for (i = 0; i < rows1; ++i)
		for (j = 0; j < cols2; ++j)
		{
			double res = 0.0;
			for (k = 0; k < cols1; ++k)
				res += a[i * cols1 + k] * b[j * rows2 + k];
			product[i * cols2 + j] = res;
		}
}
static void matrix_matrix_transpose_self_product(const double *a, double *product, unsigned int rows1, unsigned int cols1)
{
	unsigned int i, j, k;
	for (i = 0; i < rows1; ++i)
	{
		for (j = 0; j < rows1; ++j)
		{
			double res = 0.0;
			for (k = 0; k < cols1; ++k)
				res += a[i * cols1 + k] * a[j * cols1 + k];
			product[i * rows1 + j] = res;
		}
	}
}
static void transpose(double *src, double *dst, unsigned int n, unsigned int m)
{
	for (unsigned int k = 0; k < n * m; k++)
		dst[k] = src[m * (k % n) + (k / n)];
}
static void matrix_vector_mult(const double *mat, const double *vec, double *c, unsigned int rows, unsigned int cols, unsigned int stepSize)
{
	for (unsigned int i = 0; i < rows; i++)
	{
		double res = 0.0;
		for (unsigned int j = 0; j < cols; j++)
			res += (mat[i * stepSize + j] * vec[j]);
		c[i] = res;
	}
}
unsigned int cholesky(double *A, double *L, unsigned int n)
{
	unsigned int i, k;
	unsigned int r = 0;
	double tol;
	double minV = A[0];
	for (i = 1; i < n; i++)
	{
		if (A[i * n + i] > 0.0)
		{
			if (A[i * n + i] < minV)
				minV = A[i * n + i];
		}
	}
	tol = minV * 1e-9;
	double *tmp = (double*)malloc(n * sizeof(double));
	for (k = 0; k < n; k++)
	{
		matrix_vector_mult(L + k * n, L + k * n, tmp, n - k, r, n);
		for (i = k; i < n; i++)
			L[i * n + r] = A[i * n + k] - tmp[i - k];
		if (L[k * n + r] > tol)
		{
			L[k * n + r] = sqrt(L[k * n + r]);
			if (k < n - 1)
			{
				for (i = k + 1; i < n; i++)
					L[i * n + r] = L[i * n + r] / L[k * n + r];
			}
			r++;
		}
	}
	free(tmp);
	return r;
}
void inv_chol(double *L, double *Y, unsigned int n)
{
	unsigned int i, j, k;
	double *S = (double*)malloc(n * n * sizeof(double));
	memset(S, 0, n * n * sizeof(double));
	for (i = 0; i < n; i++)
		S[i * n + i] = 1.0 / L[i * n + i];
	double res;
	for (i = n; i-- > 0;)
	{
		for (j = n; j-- > 0;)
		{
			res = 0.0;
			for (k = 0; k < n - i - 1; k++)
				res += (L[(i + 1) * n + k * n + i] * Y[(i + 1) * n + k * n + j]);
			Y[i * n + j] = (S[i * n + j] - res) / L[i * n + i];
			Y[j * n + i] = Y[i * n + j];
		}
	}
	free(S);
}
void geninv(double *G, unsigned int m1, unsigned int n1, double *Y, unsigned int size[2])
{
	unsigned int i, j, k;
	char tp;
	unsigned int n = min(m1, n1);
	double *A = (double*)malloc(n * n * sizeof(double));
	if (m1 < n1)
	{
		tp = 1;
		matrix_matrix_transpose_self_product(G, A, m1, n1);
	}
	else
	{
		tp = 0;
		matrix_transpose_matrix_self_product(G, A, m1, n1);
	}
	double *L = (double*)malloc(n * n * sizeof(double));
	memset(L, 0, n * n * sizeof(double));
	unsigned int r = cholesky(A, L, n);
	double *Ltmp = (double*)malloc(r * r * sizeof(double));
	matrix_transpose_matrix_self_product_in_out_stride(L, Ltmp, n, r, n, r);
	double *L2 = (double*)malloc(r * r * sizeof(double));
	memset(L2, 0, r * r * sizeof(double));
	cholesky(Ltmp, L2, r);
	double *M = (double*)malloc(r * r * sizeof(double));
	inv_chol(L2, M, r);
	double *tmpRes1 = (double*)malloc(r * sizeof(double));
	double *tmpRes2 = (double*)malloc(r * sizeof(double));
	double *tmpRes3;
	double *tmpRes4 = (double*)malloc(n1 * m1 * sizeof(double));
	if (tp)
	{
		tmpRes3 = (double*)malloc(r * sizeof(double));
		for (i = 0; i < n1; ++i)
		{
			// tmpRes1 = G' * L
			for (j = 0; j < r; ++j)
			{
				double res = 0.0;
				for (k = 0; k < m1; ++k)
					res += G[k * n1 + i] * L[k * n + j];
				tmpRes1[j] = res;
			}
			// tmpRes2 = tmpRes1 * M
			for (j = 0; j < r; ++j)
			{
				double res = 0.0;
				for (k = 0; k < r; ++k)
					res += tmpRes1[k] * M[k * r + j];
				tmpRes2[j] = res;
			}
			// tmpRes3 = tmpRes2 * M
			for (j = 0; j < r; ++j)
			{
				double res = 0.0;
				for (k = 0; k < r; ++k)
					res += tmpRes2[k] * M[k * r + j];
				tmpRes3[j] = res;
			}
			// Y = tmpRes3 * L'
			for (j = 0; j < m1; ++j)
			{
				double res = 0.0;
				for (k = 0; k < r; ++k)
					res += tmpRes3[k] * L[j * n + k];
				Y[i * m1 + j] = res;
			}
		}
	}
	else
	{
		tmpRes3 = (double*)malloc(n1 * sizeof(double));
		for (i = 0; i < n1; ++i)
		{
			for (j = 0; j < r; ++j)
			{
				double res = 0.0;
				for (k = 0; k < r; ++k)
					res += L[i * n + k] * M[k * r + j];
				tmpRes1[j] = res;
			}
			for (j = 0; j < r; ++j)
			{
				double res = 0.0;
				for (k = 0; k < r; ++k)
					res += tmpRes1[k] * M[k * r + j];
				tmpRes2[j] = res;
			}
			for (j = 0; j < n1; ++j)
			{
				double res = 0.0;
				for (k = 0; k < r; ++k)
					res += tmpRes2[k] * L[j * n1 + k];
				tmpRes3[j] = res;
			}
			for (j = 0; j < m1; ++j)
			{
				double res = 0.0;
				for (k = 0; k < n1; ++k)
					res += tmpRes3[k] * G[j * n1 + k];
				Y[i * m1 + j] = res;
			}
		}
	}
	size[0] = n1;
	size[1] = m1;
	free(A);
	free(L);
	free(Ltmp);
	free(L2);
	free(M);
	free(tmpRes1);
	free(tmpRes2);
	free(tmpRes3);
	free(tmpRes4);
}
void qr_getSize(unsigned int m, unsigned int n, unsigned int Q_size[2], unsigned int R_size[2], unsigned int *permSize)
{
	Q_size[0] = Q_size[1] = m;
	*permSize = n;
	R_size[0] = m;
	R_size[1] = n;
}
void backwardSubstitution(double *U, unsigned int n, double *b, double *x)
{
	unsigned int j, k;
	if (U[(n - 1) * n + (n - 1)] != 0.0)
		x[n - 1] = b[n - 1] / U[(n - 1) * n + (n - 1)];
	else
		x[n - 1] = 0.0;
	double x1;
	for (k = n - 1; k-- > 0;)
	{
		if (U[k * n + k] != 0.0)
		{
			double sum = 0.0;
			for (j = k + 1; j < n; j++)
				sum += U[k * n + j] * x[j];
			x1 = 1.0 / U[k * n + k] * (b[k] - sum);
		}
		else
			x1 = 0.0;
		x[k] = x1;
	}
}
void forwardSubstitution(double *L, unsigned int n, double *b, double *x)
{
	unsigned int j, k;
	if (L[0] != 0.0)
		x[0] = b[0] / L[0];
	else
		x[0] = 0.0;
	double x1;
	for (k = 1; k < n; k++)
	{
		if (L[k * n + k] != 0.0)
		{
			double sum = 0.0;
			for (j = k; j-- > 0;)
				sum += L[k * n + j] * x[j];
			x1 = 1.0 / L[k * n + k] * (b[k] - sum);
		}
		else
			x1 = 0.0;
		x[k] = x1;
	}
}
typedef struct
{
	double *Q, *R, *Q1, *preInv, *x1, *invPR;
	int *perm;
	QR_heap qr_mem;
} ulin_mem;
void preallocate_ulinsys_qr(ulin_mem *mem, unsigned int m, unsigned int n)
{
	unsigned int Q_size[2];
	unsigned int R_size[2];
	unsigned int permSize;
	qr_getSize(m, n, Q_size, R_size, &permSize);
	mem->Q = (double*)malloc(Q_size[0] * Q_size[1] * sizeof(double));
	mem->R = (double*)malloc(R_size[0] * R_size[1] * sizeof(double));
	mem->perm = (int*)malloc(permSize * sizeof(int));
	mem->Q1 = (double*)malloc(m * m * sizeof(double));
	mem->preInv = (double*)malloc(n * n * sizeof(double));
	mem->x1 = (double*)malloc(n * sizeof(double));
	mem->invPR = (double*)malloc(n * n * sizeof(double));
	preallocateMemQR(&mem->qr_mem, m, n);
}
void free_ulinsys_qr(ulin_mem *mem)
{
	free(mem->Q);
	free(mem->R);
	free(mem->perm);
	free(mem->Q1);
	free(mem->preInv);
	free(mem->x1);
	free(mem->invPR);
	freeMemQR(&mem->qr_mem);
}
unsigned int ulinsys_qr(ulin_mem *ulinM, double *A, unsigned int m, unsigned int n, double *b, double *Q2, double *ds)
{
	QR_pivoting(&ulinM->qr_mem, A, m, n, ulinM->Q, ulinM->R, ulinM->perm);
	unsigned int i, j, k;
	const double epsi = 5e-16;
	double tol = max(n, m);
	double mV = ulinM->R[0];
	for (unsigned int i = 1; i < m * n; i++)
	{
		double res = fabs(ulinM->R[i]);
		if (res > mV)
			mV = res;
	}
	tol = tol * mV * epsi;
	unsigned int i1 = 0;
	unsigned int cnt = 0;
	for (i = 0; i < m; i++)
	{
		for (j = 0; j < n; j++)
		{
			if (fabs(ulinM->R[i * n + j]) > tol)
			{
				i1 = i;
				for (k = 0; k < m; k++)
					ulinM->Q1[k * m + cnt] = ulinM->Q[k * m + i];
				if (i > 0)
					memcpy(ulinM->R + cnt * n, ulinM->R + i * n, n * sizeof(double));
				cnt++;
				break;
			}
		}
	}
	unsigned int Q2Len = m - i1 - 1;
	for (i = 0; i < m; i++)
		for (j = 0; j < Q2Len; j++)
			Q2[i * Q2Len + j] = ulinM->Q[i * m + i1 + j + 1];
	memset(ulinM->preInv, 0, n * cnt * sizeof(double));
	for (i = 0; i < cnt; i++)
		for (j = 0; j < cnt; j++)
			ulinM->preInv[ulinM->perm[i] * cnt + j] = ulinM->R[j * n + i];
	unsigned int oS[2];
	geninv(ulinM->preInv, n, cnt, ulinM->invPR, oS);
	matrix_vector_mult(ulinM->invPR, b, ulinM->x1, cnt, n, n);
	for (i = 0; i < m; i++)
	{
		double sum = 0.0;
		for (j = 0; j < cnt; j++)
			sum += ulinM->Q1[i * m + j] * ulinM->x1[j];
		ds[i] = sum;
	}
	return Q2Len;
}
typedef struct
{
	unsigned int mOpt, numCon, constraintMatSize;
	int *positionQ, *positionQQ;
	char *correspondingValueQ, *correspondingValueQQ;
} constraintsSparseMtx;
void eqconPrecompute(unsigned int L, char *C, constraintsSparseMtx *sparseMtx)
{
	unsigned int i, j;
	unsigned int Lh = L >> 1;
	unsigned int *activeConstraint = (unsigned int*)malloc(L * sizeof(unsigned int));
	unsigned int *opCnts = (unsigned int*)malloc(L * sizeof(unsigned int));
	unsigned int numCon = 0;
	char *U0l = (char*)malloc(L * Lh * sizeof(char));
	for (unsigned int actL = 0; actL < L; actL++)
	{
		unsigned char allzero = 1;
		memset(U0l, 0, L * Lh * sizeof(char));
		unsigned int mmax = ((L - 1) - actL) + 1;
		for (i = 0; i < mmax; i++)
		{
			unsigned int i1 = i + actL;
			if (i1 < Lh)
			{
				U0l[i * Lh + i1] = C[actL * L + i];
				if (sabs8(C[actL * L + i]) > 0)
					allzero = 0;
			}
			else
			{
				U0l[i * Lh + ((L - i1) - 1)] = C[actL * L + i];
				if (sabs8(C[actL * L + i]) > 0)
					allzero = 0;
			}
		}
		for (i = 0; i < Lh; i++)
		{
			for (j = 0; j < Lh; j++)
				U0l[i * Lh + j] += U0l[(L - i - 1) * Lh + j];
		}
		if (!allzero)
		{
			activeConstraint[numCon] = actL;
			unsigned int numOpt = 0;
			for (i = 0; i < Lh; i++)
				if (sabs8(U0l[i * Lh + 0]) > 0)
					numOpt++;
			unsigned int mO = numOpt;
			for (j = 1; j < Lh; j++)
			{
				numOpt = 0;
				for (i = 0; i < Lh; i++)
				{
					if (sabs8(U0l[i * Lh + j]) > 0)
						numOpt++;
				}
				if (numOpt > mO)
					mO = numOpt;
			}
			opCnts[numCon] = mO;
			//printf("%d %d %d\n", numCon + 1, actL, mO);
			numCon++;
		}
	}
	// Retrieve sparse matrix position
	unsigned int mOpt = opCnts[0];
	for (i = 1; i < numCon; i++)
	{
		if (opCnts[i] > mOpt)
			mOpt = opCnts[i];
	}
	unsigned int constraintMatSize = Lh * mOpt;
	sparseMtx->positionQ = (int*)malloc(numCon * constraintMatSize * sizeof(int));
	sparseMtx->positionQQ = (int*)malloc(numCon * constraintMatSize * sizeof(int));
	for (unsigned int ii = 0; ii < numCon * constraintMatSize; ii++)
	{
		sparseMtx->positionQ[ii] = -1;
		sparseMtx->positionQQ[ii] = -1;
	}
	sparseMtx->correspondingValueQ = (char*)malloc(numCon * constraintMatSize * sizeof(char));
	sparseMtx->correspondingValueQQ = (char*)malloc(numCon * constraintMatSize * sizeof(char));
	memset(sparseMtx->correspondingValueQ, 0, numCon * constraintMatSize * sizeof(char));
	memset(sparseMtx->correspondingValueQQ, 0, numCon * constraintMatSize * sizeof(char));
	char *QQ = (char*)malloc(Lh * Lh * sizeof(char));
	for (unsigned int ii = 1; ii < numCon; ii++)
	{
		unsigned int actL = activeConstraint[ii];
		memset(U0l, 0, L * Lh * sizeof(char));
		unsigned int mmax = ((L - 1) - actL) + 1;
		for (i = 0; i < mmax; i++)
		{
			unsigned int i1 = i + actL;
			if (i1 < Lh)
				U0l[i * Lh + i1] = C[actL * L + i];
			else
				U0l[i * Lh + ((L - i1) - 1)] = C[actL * L + i];
		}
		for (i = 0; i < Lh; i++)
		{
			for (j = 0; j < Lh; j++)
				U0l[i * Lh + j] += U0l[(L - i - 1) * Lh + j];
		}
		for (i = 0; i < Lh; i++)
		{
			for (j = 0; j < Lh; j++)
				QQ[i * Lh + j] = U0l[i * Lh + j] + U0l[j * Lh + i];
		}
		unsigned int n1, n2;
		for (i = 0; i < Lh; i++)
		{
			n1 = 0; n2 = 0;
			for (j = 0; j < Lh; j++)
			{
				if (sabs8(U0l[j * Lh + i]) > 0)
				{
					sparseMtx->positionQ[constraintMatSize * ii + i * mOpt + n1] = j;
					if (U0l[j * Lh + i] == -2)
						sparseMtx->correspondingValueQ[constraintMatSize * ii + i * mOpt + n1] = 0;
					else if (U0l[j * Lh + i] == -1)
						sparseMtx->correspondingValueQ[constraintMatSize * ii + i * mOpt + n1] = 1;
					else if (U0l[j * Lh + i] == 0)
						sparseMtx->correspondingValueQ[constraintMatSize * ii + i * mOpt + n1] = 2;
					else if (U0l[j * Lh + i] == 1)
						sparseMtx->correspondingValueQ[constraintMatSize * ii + i * mOpt + n1] = 3;
					else if (U0l[j * Lh + i] == 2)
						sparseMtx->correspondingValueQ[constraintMatSize * ii + i * mOpt + n1] = 4;
					else
						printf("Shit 1!\n");
					n1++;
				}
				if (sabs8(QQ[j * Lh + i]) > 0)
				{
					sparseMtx->positionQQ[constraintMatSize * ii + i * mOpt + n2] = j;
					if (QQ[j * Lh + i] == -2)
						sparseMtx->correspondingValueQQ[constraintMatSize * ii + i * mOpt + n2] = 0;
					else if (QQ[j * Lh + i] == -1)
						sparseMtx->correspondingValueQQ[constraintMatSize * ii + i * mOpt + n2] = 1;
					else if (QQ[j * Lh + i] == 0)
						sparseMtx->correspondingValueQQ[constraintMatSize * ii + i * mOpt + n2] = 2;
					else if (QQ[j * Lh + i] == 1)
						sparseMtx->correspondingValueQQ[constraintMatSize * ii + i * mOpt + n2] = 3;
					else if (QQ[j * Lh + i] == 2)
						sparseMtx->correspondingValueQQ[constraintMatSize * ii + i * mOpt + n2] = 4;
					else
						printf("Shit 2!\n");
					n2++;
				}
			}
		}
	}
	sparseMtx->numCon = numCon;
	sparseMtx->mOpt = mOpt;
	sparseMtx->constraintMatSize = constraintMatSize;
	free(QQ);
	free(opCnts);
	free(activeConstraint);
	free(U0l);
}
void genC(unsigned int N, unsigned int L, char *C)
{
	unsigned int i, j;
	unsigned int N2 = N << 1;
	memset(C, 0, L * L * sizeof(char));
	for (i = 0; i < L; i++)
	{
		if (i % N2 == 0)
		{
			if ((i / N2) % 2 == 0)
			{
				for (j = 0; j < L; j++)
					C[i * L + j] = 1;
			}
			else
			{
				for (j = 0; j < L; j++)
					C[i * L + j] = -1;
			}
		}
		for (j = 0; j < L; j++)
		{
			if ((2 * j) > (N + L - i))
			{
				if ((2 * j - N - L + 1 + i) % N2 == 0)
				{
					if (((2 * j - N - L + 1 + i) / N2) % 2 == 0)
						C[i * L + j] += 1;
					else
						C[i * L + j] -= 1;
				}
			}
			else
			{
				if ((N + L - 2 * j - 1 - i) % N2 == 0)
				{
					if (((N + L - 2 * j - 1 - i) / N2) % 2 == 0)
						C[i * L + j] += 1;
					else
						C[i * L + j] -= 1;
				}
			}
		}
	}
}
void genP(int L, double fc, double df, double *P)
{
	int ii, jj;
	int Lh = L >> 1;
	double theta_s = M_PI * (fc + df);
	double d = 2.0 * (M_PI - theta_s);
	const double delta_s = 1e-14;
	for (ii = 0; ii < Lh; ii++)
	{
		P[ii * Lh + ii] = d - 2.0 * sin(theta_s * (double)(2 * (ii + 1) - L - 1)) / (double)(2 * (ii + 1) - L - 1) + delta_s;
		for (jj = ii + 1; jj < Lh; jj++)
		{
			double idx1 = (ii + 1) - (jj + 1);
			double idx2 = (ii + 1) + (jj + 1);
			P[ii * Lh + jj] = -2.0 * (sin(theta_s * idx1) / idx1 + sin(theta_s * (double)(idx2 - L - 1)) / (double)(idx2 - L - 1));
			P[jj * Lh + ii] = P[ii * Lh + jj];
		}
	}
}
void eqconTransposeB(double *x, unsigned int N, unsigned int L, constraintsSparseMtx *mtx, double gain, double *A, double *b)
{
	unsigned int i, j;
	double N2 = (double)(N << 1);
	unsigned int Lh = L >> 1;
	for (i = 0; i < Lh; i++)
		A[i * mtx->numCon + 0] = x[i] * N2;
	const double lut[5] = { -2, -1, 0, 1, 2 };
	double res2, sumV1, sumV2;
	res2 = 0.0;
	for (i = 0; i < Lh; i++)
		res2 += x[i] * x[i];
	b[0] = (1.0 / N) - (res2 * N);
	for (unsigned int cont = 1; cont < mtx->numCon; cont++)
	{
		res2 = 0.0;
		for (i = 0; i < Lh; i++)
		{
			sumV1 = 0.0;
			sumV2 = 0.0;
			for (j = 0; j < mtx->mOpt; j++)
			{
				int pos = mtx->positionQQ[mtx->constraintMatSize * cont + i * mtx->mOpt + j];
				if (pos >= 0)
					sumV1 = sumV1 + lut[mtx->correspondingValueQQ[mtx->constraintMatSize * cont + i * mtx->mOpt + j]] * x[pos];
				pos = mtx->positionQ[mtx->constraintMatSize * cont + i * mtx->mOpt + j];
				if (pos >= 0)
					sumV2 = sumV2 + lut[mtx->correspondingValueQ[mtx->constraintMatSize * cont + i * mtx->mOpt + j]] * x[pos];
			}
			A[i * mtx->numCon + cont] = sumV1 * gain;
			res2 = res2 + sumV2 * x[i];
		}
		b[cont] = -res2 * gain;
	}
}
double bessi0(double x)
{
	double xh, sum, pow, ds;
	int k;
	xh = 0.5 * x;
	sum = 1.0;
	pow = 1.0;
	k = 0;
	ds = 1.0;
	while (ds > sum * DBL_EPSILON)
	{
		++k;
		pow = pow * (xh / k);
		ds = pow * pow;
		sum = sum + ds;
	}
	return sum;
}
double getKaiser(double x, double alpha)
{
	static double alpha_prev = 0.0;
	static double Ia = 1.0;
	double beta, win;
	if (alpha != alpha_prev)
	{
		Ia = bessi0(alpha);
		alpha_prev = alpha;
	}
	if (x <= -1.0 || x >= 1.0)
		win = 0.0;
	else
	{
		beta = alpha * sqrt(1.0 - x * x);
		win = bessi0(beta) / Ia;
	}
	return win;
}
void halfSincKaiser(double *win, int L, double fc, double alpha)
{
	int i, minus1 = L - 1;
	for (i = 0; i < L >> 1; ++i)
	{
		double filter = fc * sinc(fc * ((0.5 + i) - L * 0.5));
		win[i] = filter * getKaiser((double)(2 * i - minus1) / minus1, alpha);
	}
}
void subsamplingCal(unsigned int M, double alpha, double *f_def, unsigned int *Sk)
{
	unsigned int i, k;
	for (i = 0; i < M + 1; i++)
	{
		double w = 2.0 * M_PI * (i / (double)(2 * M));
		f_def[i] = (w - 2.0 * atan((alpha * sin(w)) / (alpha * cos(w) - 1))) / (2.0 * M_PI);
	}
	double a_def = 2.0 * M_PI * (1.0 / (double)(2 * M)) + M_PI / (double)(2 * M);
	double fs_def = (a_def - 2.0 * atan(alpha * sin(a_def) / (alpha * cos(a_def) - 1.0))) / (2.0 * M_PI);
	double fU, fL;
	for (k = 0; k < M; k++)
	{
		Sk[k] = 1;
		if (k == 0)
		{
			fU = fs_def;
			fL = f_def[k];
		}
		else if (k == (M - 1))
		{
			fU = f_def[k + 1];
			fL = f_def[k - 1];
		}
		else
		{
			fU = f_def[k + 2];
			fL = f_def[k - 1];
		}
		double B = fU - fL;
		unsigned int nk_max = (unsigned int)(fU / B);
		for (unsigned int nk = 0; nk < nk_max; nk++)
		{
			unsigned int ub = (unsigned int)((nk + 1) / (2 * fU));
			unsigned int lb = (unsigned int)max(ceil(nk / (2 * fL)), 1.0) - 1;
			if (ub >= lb)
			{
				for (unsigned int s = lb; s < ub; s++)
				{
					if ((s + 1.0) * B <= 0.5)
						Sk[k] = max(Sk[k], (s + 1));
				}
			}
		}
	}
}
typedef struct
{
	unsigned long long inuse, capacity, grow_num;
	double *data;
} sample_vector;
void init_sample_vector(sample_vector *s, int init_capacity, int grow)
{
	s->data = (double*)malloc(init_capacity * sizeof(double));
	s->inuse = 0;
	s->capacity = init_capacity;
	s->grow_num = grow;
}
void push_back_sample_vector(sample_vector *s, double *x, int lenS)
{
	if ((s->inuse + lenS + (s->grow_num >> 1)) > s->capacity)
	{
		s->capacity += (s->grow_num + lenS);
		s->data = (double*)realloc(s->data, s->capacity * sizeof(double));
	}
	memcpy(s->data + s->inuse, x, lenS * sizeof(double));
	s->inuse += lenS;
}
void clear_sample_vector(sample_vector *s)
{
	s->inuse = 0;
}
void free_sample_vector(sample_vector *s)
{
	free(s->data);
}
float* allpass_char(double alpha, unsigned int L, unsigned int *CFiltLen)
{
	unsigned int i;
	double *allpass_delay_chain = (double*)malloc(L * sizeof(double));
	memset(allpass_delay_chain, 0, L * sizeof(double));
	double *APC_delay_1 = (double*)malloc(L * sizeof(double));
	double *APC_delay_2 = (double*)malloc(L * sizeof(double));
	memset(APC_delay_1, 0, L * sizeof(double));
	memset(APC_delay_2, 0, L * sizeof(double));
	unsigned int n = 0;
	sample_vector smpVec;
	init_sample_vector(&smpVec, 2048, 2048);
	double Threshold = 2e-3;
	unsigned int ResInd1 = 0, ResInd2 = 0;
	double pwr = 0.0f;
	while (1)
	{
		double R1_a;
		if (n == 0)
			R1_a = 1.0f;
		else
			R1_a = 0.0f;
		double R2_a = allpass_delay_chain[0];
		allpass_delay_chain[0] = R1_a;
		for (i = 1; i < L; i++)
		{
			double R3_a = R2_a;
			R2_a = allpass_delay_chain[i];
			R1_a = (R2_a - R1_a) * alpha + R3_a;
			allpass_delay_chain[i] = R1_a;
		}
		double R1_b = allpass_delay_chain[0];
		for (i = 0; i < L - 1; i++)
		{
			double R3_b = APC_delay_1[i];
			APC_delay_1[i] = R1_b;
			double R2_b = APC_delay_2[i];
			R1_b = (R2_b - R1_b) * alpha + R3_b;
			APC_delay_2[i] = R1_b;

			R1_b = R1_b + allpass_delay_chain[i + 1];
		}
		push_back_sample_vector(&smpVec, &R1_b, 1);
		double curPwr = fabs(R1_b);
		if (curPwr > Threshold && !ResInd1)
			ResInd1 = n;
		if (n == 0)
			pwr = curPwr;
		else
			pwr = curPwr * 0.5 + pwr * 0.5;
		if (curPwr < Threshold && (pwr < Threshold) && (ResInd1 > ResInd2))
		{
			ResInd2 = n;
			break;
		}
		n++;
	}
	if (ResInd2 > ResInd1)
		*CFiltLen = ResInd2 - ResInd1;
	else
	{
		*CFiltLen = 0;
		return 0;
	}
	float *C = (float*)malloc(*CFiltLen * sizeof(float));
	for (i = ResInd2; i-- > ResInd1; )
		C[ResInd2 - i - 1] = (float)(smpVec.data[i] / (double)L);
	free_sample_vector(&smpVec);
	free(allpass_delay_chain);
	free(APC_delay_1);
	free(APC_delay_2);
	return C;
}
void cos_fib_paraunitary1(unsigned int N, unsigned int m, unsigned int L, double df, double *h_opt)
{
	unsigned int i, k;
	unsigned int Lh = L >> 1;
	double fc = 1.21 / (double)(2 * N);
	double as = 7.95 + 7.18 * 2.0 * L * df;
	double beta;
	if (as < 21.0)
		beta = 0.0;
	else if (as < 50.0)
		beta = 0.58417 * pow(as - 20.96, 0.4) + 0.07886 * (as - 20.96);
	else
		beta = 0.1102 * (as - 8.7);
	double *h = (double*)malloc(Lh * sizeof(double));
	halfSincKaiser(h, L, fc, beta);
	double *P = (double*)malloc(Lh * Lh * sizeof(double));
	double *PT = (double*)malloc(Lh * Lh * sizeof(double));
	genP((int)L, fc, df, P);
	transpose(P, PT, Lh, Lh);
	free(P);
	double a = N * 0.5;
	char *C = (char*)malloc(L * L * sizeof(char));
	genC(N, L, C);
	constraintsSparseMtx sparseMtx;
	eqconPrecompute(L, C, &sparseMtx);
	unsigned int Kiter = 200;
	unsigned int Kiter1 = 40;
	double epsil = 1e-10;
	double epsil1 = 1e-7;
	char flag = 0;
	double previous2 = 0;
	double previous1 = 0;
	char problematic = 0;
	double *dt = (double*)malloc(Lh * sizeof(double));
	double *A = (double*)malloc(Lh * sparseMtx.numCon * sizeof(double));
	double *b = (double*)malloc(sparseMtx.numCon * sizeof(double));
	double *ds = (double*)malloc(Lh * sizeof(double));
	double *Q2 = (double*)malloc(Lh * Lh * sizeof(double));
	double *Q2T = (double*)malloc(Lh * Lh * sizeof(double));
	double *P1 = (double*)malloc(Lh * Lh * sizeof(double));
	double *P1Q2_mult = (double*)malloc(Lh * Lh * sizeof(double));
	double *R = (double*)malloc(Lh * Lh * sizeof(double));
	double *RT = (double*)malloc(Lh * Lh * sizeof(double));
	double *phi = (double*)malloc(Lh * sizeof(double));
	double *d = (double*)malloc(Lh * sizeof(double));
	ulin_mem ulinM;
	preallocate_ulinsys_qr(&ulinM, Lh, sparseMtx.numCon);
	for (k = 0; k < Kiter; k++)
	{
		double norm_ht = 0.0;
		for (i = 0; i < Lh; i++)
			norm_ht += h[i] * h[i];
		norm_ht = sqrt(norm_ht);
		eqconTransposeB(h, N, L, &sparseMtx, a, A, b);
		unsigned int dim2_Q2 = ulinsys_qr(&ulinM, A, Lh, sparseMtx.numCon, b, Q2, ds);
		if (k > 1 && flag == 0)
		{
			if (fabs(previous2 - previous1) < epsil1)
				flag = 1;
		}
		if (flag || k > Kiter1)
			memcpy(dt, ds, Lh * sizeof(double));
		else
		{
			for (i = 0; i < Lh; i++)
				dt[i] = ds[i] + h[i];
		}
		transpose(Q2, Q2T, Lh, dim2_Q2);
		matrix_transposedMatrix_product(Q2T, PT, P1, dim2_Q2, Lh, Lh, Lh);
		matrix_transposedMatrix_product(P1, Q2T, P1Q2_mult, dim2_Q2, Lh, Lh, dim2_Q2);
		memset(R, 0, dim2_Q2 * dim2_Q2 * sizeof(double));
		cholesky(P1Q2_mult, R, dim2_Q2);
		matrix_vector_mult(P1, dt, P1Q2_mult, dim2_Q2, Lh, Lh);
		forwardSubstitution(R, dim2_Q2, P1Q2_mult, d);
		transpose(R, RT, dim2_Q2, dim2_Q2);
		backwardSubstitution(RT, dim2_Q2, d, phi);
		matrix_vector_mult(Q2, phi, d, Lh, dim2_Q2, dim2_Q2);
		for (i = 0; i < Lh; i++)
			d[i] = ds[i] - d[i];
		double norm_d = 0.0, sum_d = 0.0;
		double max_d = d[0];
		for (i = 0; i < Lh; i++)
		{
			double abs_d = fabs(d[i]);
			sum_d += abs_d;
			norm_d += d[i] * d[i];
			if (abs_d > max_d)
				max_d = abs_d;
		}
		sum_d = sum_d / Lh;
		norm_d = sqrt(norm_d);
		if (sum_d > 1.0)
		{
			if (!problematic)
			{
				for (i = 0; i < Lh; i++)
					h[i] = d[i] / max_d * fc;
				problematic = 1;
				continue;
			}
			printf("Optimization go wrong, df parameter probably too large\n");
			memset(h_opt, 0, L * sizeof(double));
		}
		for (i = 0; i < Lh; i++)
			h[i] += d[i];
		previous2 = previous1;
		previous1 = norm_d / norm_ht;
		if (previous1 < epsil)
			break;
	}
	double totalPwr = 0.0;
	for (i = 0; i < Lh; i++)
	{
		h_opt[i] = h[i];
		h_opt[L - i - 1] = h[i];
		totalPwr += h_opt[i];
	}
	totalPwr = totalPwr * 2.0;
	double squareRt_N = sqrt(N);
	double norhpt = 0.0;
	if (fabs(totalPwr) > 0.0)
	{
		for (i = 0; i < L; i++)
		{
			h_opt[i] = squareRt_N * h_opt[i] / totalPwr;
			norhpt += h_opt[i] * h_opt[i];
		}
	}
	else
	{
		for (i = 0; i < L; i++)
		{
			h_opt[i] = squareRt_N * h_opt[i];
			norhpt += h_opt[i] * h_opt[i];
		}
	}
	norhpt = sqrt(norhpt);
	const double squareRt_2 = sqrt(2.0);
	if (norhpt > DBL_EPSILON)
	{
		for (i = 0; i < L; i++)
			h_opt[i] = squareRt_2 * h_opt[i] / norhpt;
	}
	else
	{
		for (i = 0; i < L; i++)
			h_opt[i] = squareRt_2 * h_opt[i];
	}
	free(h);
	free(PT);
	free(C);
	free(dt);
	free(A);
	free(b);
	free(ds);
	free(Q2);
	free(Q2T);
	free(P1);
	free(P1Q2_mult);
	free(R);
	free(RT);
	free(phi);
	free(d);
	free(sparseMtx.positionQ);
	free(sparseMtx.positionQQ);
	free(sparseMtx.correspondingValueQ);
	free(sparseMtx.correspondingValueQQ);
	free_ulinsys_qr(&ulinM);
}
double getOptimalDF(unsigned int N, unsigned int m)
{
	if (m < 1)
		return 0.0;
	else if (m >= 1 && m < 7)
	{
		if (N < 97)
		{
			const double dfLUT[6 * 96] = { 1,1,1,1,1,1,0.286535730580963,0.313405026255228,0.302302065670699,0.304931433419360,0.304207267806462,0.295200983222171,0.280404534944377,0.261993361638258,0.188621251756131,0.243277852771489,0.215230217928873,0.200555709104807,0.222144343862097,0.183881439834043,0.126486068103919,0.152837046255515,0.146553911685564,0.143949498674247,0.201765257153999,0.156756588011550,0.0966713345320333,0.126838214621722,0.119759709102859,0.118108415531563,0.182835849707188,0.136094257302092,0.0801057846520175,0.103135778085260,0.0987321746164191,0.0961898377760306,0.198584536763086,0.143730288458311,0.0664170631394259,0.104216312645053,0.0915523406788585,0.0843631151227219,0.167637759878204,0.119383345099339,0.0583727711411420,0.0929865590348050,0.0940219341422456,0.0756257255323145,0.148852790219506,0.100002070784993,0.0508146136068421,0.0750377176002302,0.0702369292348170,0.0675945851189043,0.138803897113211,0.111408414343653,0.0454900280641910,0.0725517536658211,0.0644016234320574,0.0584956389004383,0.126793055589455,0.0908866115465545,0.0411321307334058,0.0692738289380772,0.0708437443424617,0.0548992042850836,0.114285729504047,0.0788135480152052,0.0376176771970332,0.0565434040652236,0.0527665716497745,0.0476913976908450,0.119300694235864,0.147886072362769,0.0337555209440450,0.0582208546838003,0.0505924058911284,0.0460259804262407,0.118988253910350,0.0726684117385397,0.0315050050623462,0.0519481778699604,0.0485046047216217,0.0483969709491359,0.0974894309874196,0.0855519921440915,0.0292154816696315,0.0479901709588337,0.0430890658863303,0.0393903321276035,0.0909267573069019,0.0702170867730463,0.0278219527702299,0.0689434674781626,0.0452192374939217,0.0388722228234852,0.0866758627954328,0.0620812871786083,0.0260113460316605,0.0421830382385265,0.0387943082685463,0.0358006788114896,0.0795932637147764,0.0749930690534258,0.0246725701760771,0.0433951600384139,0.0377430654462939,0.0332792408636930,0.0766025310809220,0.0588704044841320,0.0235955925399967,0.0398907767801552,0.0370076106787303,0.0362398866871031,0.0713440757573736,0.0665466025179797,0.0221191961227829,0.0373217610695905,0.0332517421729284,0.0295270902938340,0.0686008814759274,0.0576770399960665,0.0209268813769427,0.0536315379976979,0.0380812835703351,0.0296019937192688,0.0674915852819815,0.0506378518319878,0.0198819503906200,0.0327970802383811,0.0302082446922441,0.0280565769070661,0.0621814797644564,0.0548947441408895,0.0194334102048846,0.0511925490209149,0.0304522618273720,0.0264489351317071,0.0596115816127902,0.0498354302023666,0.0183720815752653,0.0315552905431255,0.0289393985195708,0.0255784922741505,0.0566968747918586,0.0498746350578228,0.0175928038444230,0.0309626587204910,0.0271979331268442,0.0247771235904147,0.0561000678350336,0.0482263370072755,0.0167509047906884,0.0306836298681903,0.0295136904797102,0.0301887161817878,0.0553833077786568,0.0469881444091836,0.0161950300918126,0.0280696633948006,0.0248654019486431,0.0212116621765097,0.0517212345310446,0.0450108974439320,0.0156613623531573,0.0393065059110886,0.0268695996550433,0.0242034874632327,0.0534247005600975,0.0441303928731280,0.0151212532179802,0.0261282130636768,0.0236273818759760,0.0211302238716508,0.0483247677033497,0.0430018047708304,0.0145408386436545,0.0389057869650058,0.0235949004376349,0.0206741521110715,0.0460210649897785,0.0419087545569386,0.0142782233468975,0.0253619334872626,0.0234076370670243,0.0229241870795940,0.0461003236094212,0.0395260295924286,0.0138448301795670,0.0246822971153225,0.0216397682610892,0.0195688253502099,0.0430752171698957,0.0381070522560722,0.0132563109840366,0.0345561822741686,0.0252006608925133,0.0229907424565463,0.0420830900942783,0.0365552745908592,0.0128910289829759,0.0222236005196593,0.0200912973461312,0.0183283745143871,0.0404258268846344,0.0365844581595456,0.0126340879725718,0.0324620639413316,0.0216805808082700,0.0180141832282019,0.0401191255843743,0.0352586967032804,0.0122055803112268,0.0217208130239961,0.0195478210298138,0.0175157240055103,0.0387476690155897,0.0340665244338038,0.0119338595009010,0.0314754559588272,0.0194926209695145,0.0169808039049446,0.0384534992241625,0.0336010168572037,0.0115774103251644,0.0301973391903957,0.0196979848616612,0.0199931908959458,0.0366832946799787,0.0321486868378579,0.0112530813828470,0.0296923779654012,0.0180146476430734,0.0169568466942015,0.0376917074508066,0.0333351387401858,0.0109073039819467,0.0285258346319584,0.0216457882107305,0.0195064706605696,0.0349950765396196,0.0312712872627659,0.0107184037127667,0.0188700465091009,0.0169935593644321,0.0152685906179230,0.0347179519520870,0.0315213608941155,0.0104559536243444,0.0274666100908269,0.0184395403030014,0.0163684322340871,0.0336271410154620,0.0291546936851201,0.0103382049936376,0.0185273391569489,0.0167610779193276,0.0161827584138184,0.0325404660477993,0.0284934843283116,0.00994925278678341,0.0264597128234477,0.0165792404731140,0.0142159566466314,0.0324828020692176,0.0285033038569324,0.00977700528508094,0.0270761639351920,0.0167757691421617,0.0169750710334952,0.0322739927661327,0.0278115650366688,0.00957783634064658,0.0251800527138816,0.0154847280515556,0.0134191397874395,0.0315802746556976,0.0271900662806948,0.00937421122322141,0.0242958172194686,0.0185562861131863,0.0166022273247081,0.0312562190010846,0.0270215062139014,0.00915838218747557,0.0163165469198270,0.0146791059254116,0.0141442071804363,0.0298957702091218,0.0264584454897542,0.00893763985312985,0.0234934911541286,0.0162397889518126,0.0131374548617133,0.0286473032887305,0.0260229195182037,0.00873698828934518,0.0230109237844625,0.0146006271964759,0.0127700594716558,0.0290294749247222,0.0249593538030056,0.00867029711350995,0.0228426874813668,0.0146974224234290,0.0125358989841179,0.0268052565957549,0.0239295755567404,0.00849973696922565,0.0220848674285994,0.0148034604325694,0.0150992510802916,0.0273257972179152,0.0238732853901964,0.00828111610789272,0.0218637683650602,0.0137996482106061,0.0132717511190369,0.0268412743477370,0.0232957497439944,0.00811092570491636,0.0211768926560976,0.0159703503951248,0.0150210748760231,0.0264272930333729,0.0226365854870749,0.00795619024743068,0.0210289055839998,0.0129720516046906,0.0113281048915063,0.0257925287507495,0.0226267174591728,0.00791979967408489,0.0204434676635900,0.0148377174540664,0.0133472961116309,0.0251813729703272,0.0223978475960224,0.00775442620409063,0.0201873550730969,0.0130807748333761,0.0128714730072783,0.0246128432812693,0.0215448585852668,0.00756535854952169,0.0201037340175707,0.0132437124506879,0.0108039060755587,0.0244935100772315,0.0216641890069636,0.00741538122606615,0.0194715579078440,0.0131800670919955,0.0132084316810890,0.0239428972422362,0.0209952915105509,0.00729263372843204,0.0193285795993192,0.0123033085490358,0.0113176323171634,0.0245211722248032,0.0214426303597227,0.00714609483115586,0.0187678880794944,0.0139200308313409,0.0137049328304858,0.0229785596516036,0.0203006402545530,0.00707189676435824,0.0186619040220006,0.0117488562146934,0.0100923693498908,0.0222495886582748,0.0196189841166998,0.00694893798432048,0.0180809386116389,0.0135660728653243,0.0123379099879740,0.0224165111086159,0.0196292804984855,0.00680511470023437,0.0179829414306312,0.0116433609244045,0.0116712076502994,0.0230064806385155,0.0195685751309968,0.00679550308720253,0.0179921611709940,0.0120520573523409,0.0112648557735797,0.0221473701118188,0.0190259804901316,0.00662714736728971,0.0174153217434285,0.0118792410922819,0.0119463491043253,0.0222284233850171,0.0189569986593411,0.00659876314204988,0.0173171433797013,0.0113092848917238,0.0102595442961945,0.0213679756007036,0.0187145422153635,0.00641933910137340,0.0168533689776484,0.0124494651592826,0.0134442873502279,0.0203410947351749,0.0179671439820678,0.00641702303308976,0.0167765182995985,0.0107392583872162,0.0100418441272656,0.0205769717205021,0.0180606188359185,0.00632732128105611,0.0162977093677025,0.0128731652563376,0.0112899169016350,0.0200045997064156,0.0178236285710944,0.00618559453434285,0.0162132324578271,0.0106695365006391,0.0106414388547316,0.0195745479734150,0.0176128666775710,0.00608983318444651,0.0162886103442102,0.0111788312770270,0.0108527591423869,0.0197570755957475,0.0173210147889876,0.00603664177145397,0.0157481229195956,0.0107667843480706,0.0115694694293289,0.0192072932602050,0.0167523402401350,0.00598162408703552,0.0156944666493778,0.0104500315500638,0.00869918752702094,0.0192549279334858,0.0170630823247449,0.00587221126171008,0.0152963708556715,0.0112851434557191,0.0138985326929529,0.0190680841199657,0.0164630206102400,0.00578500096563830,0.0152418605069725,0.00994032609336375,0.00893497288207774,0.0187527577946502,0.0163215195784633,0.00568021279274697,0.0148546007321515,0.0156536292884337,0.00992054854734246,0.0187090781641469,0.0162902841157773,0.00566118487270783,0.0147605516699547,0.00972651308232302,0.00825660589216017,0.0187483932210098,0.0161582892160823,0.00560498673393789,0.0148461734732133,0.0105441389394452,0.00892087236087181,0.0178505207421212,0.0156510530475579,0.00546082706147610,0.0143738700870872,0.00988934517049537,0.00837412074470527,0.0179444551545752,0.0155057338755742,0.00542455701872545,0.0143583932631219,0.00989154852842195,0.00806284097025337,0.0172482277160552,0.0152929092234910,0.00540760088922871,0.0139927346859935,0.0104531002163711,0.00744578728671298,0.0177806738954098,0.0151719699134844,0.00529528554290738,0.0139650059723539,0.00930007346079130,0.00763940824581528,0.0173532574002316,0.0150274909106590,0.00523180404821391,0.0136161213211966,0.0143783219886104,0.00735476634393024,0.0168799675007447,0.0147189452861415,0.00512531469177738,0.0135451646341051,0.00909889303730167,0.00717687259470160,0.0170771027793238,0.0147717674540675,0.00514683407197119,0.0136040282544694,0.0125076141494779,0.00746052235113134,0.0168230280199414,0.0148239278438290,0.00505190246210728,0.0132183426339741,0.00942601213621858,0.00720259707621030,0.0162092092541085,0.0142883357445182,0.00501555048598031,0.0132367606091543,0.00947719004941255,0.00671070233930744,0.0162995657285549,0.0143682762715256,0.00497211226538670,0.0129004248190315,0.0135404538167744,0.00689752304126516,0.0157716686338333,0.0138468550712362,0.00485397761131488,0.0128862290549038,0.00850895839029396,0.00682709827243405,0.0157853313797894,0.0138499898370057,0.00485581388272643,0.0125804369192988,0.0132367503746168,0.00646499960640746,0.0160563860862572,0.0138618485883055,0.00479332821428841,0.0125121148569109,0.00794540602773179,0.00661201855731945,0.0158426632503910,0.0135727909628957,0.00477185318153346,0.0121423625325515,0.00778485240589310,0.00615822433136518,0.0155706950025644,0.0135953509544725,0.00471722471867608,0.0122130724904145,0.00758345101085547,0.00644726598517334,0.0152441240400296,0.0131690267212254,0.00468388252938771,0.0120765605034441,0.00756642530757339,0.00621613798344276,0.0149536804825575,0.0130386566282805,0.00456550569208802,0.0118711857732349,0.00740334112425530,0.00634801147615309 };
			return dfLUT[(N - 1) * 6 + (m - 1)];
		}
		else
		{
			if (m == 1)
				return 0.2892 * exp(-0.11294 * N) + 0.054895 * exp(-0.013092 * N);
			else if (m == 2)
				return 0.37177 * exp(-0.27004 * N) + 0.08958 * exp(-0.021978 * N);
			else if (m == 3)
				return 0.6658 * exp(-0.4844 * N) + 0.04553 * exp(-0.030793 * N);
			else if (m == 4)
				return 0.50656 * exp(-0.36419 * N) + 0.061509 * exp(-0.01904 * N);
			else if (m == 5)
				return 0.51554 * exp(-0.38706 * N) + 0.060656 * exp(-0.025357 * N);
			else if (m == 6)
				return 0.51908 * exp(-0.40187 * N) + 0.057579 * exp(-0.027425 * N);
		}
	}
	else
		return 1.0 / (1.5 * max(N, m));
}

int main(void){double h[20];cos_fib_paraunitary1(5,2,20,getOptimalDF(5,2),h);for(int i=0;i<20;i++)printf("%a%s",h[i],i==19?"\n":",");return 0;}
