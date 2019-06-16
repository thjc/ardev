/* -- 2007-05-07 -- 
 * libthjc - utility library
 *  Copyright 2005-2007 - Toby Collett (ardev _at_ plan9.net.nz)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 *
 */
#include <libthjc/matrix.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

unsigned int Matrix::WhichCol = 0;

Matrix::Matrix(unsigned int Rows, unsigned int Cols)
{
	rows = Rows;
	cols = Cols;
	data = new double[Cols*Rows];
	LUData = NULL;

}

Matrix::Matrix(unsigned int Rows, unsigned int Cols, unsigned int Type)
{
	LUData = NULL;

	rows = Rows;
	cols = Cols;
	data = new double[Cols*Rows];
	
	if (Type == 0 || Type == 1)
		for (unsigned long i=0; i<rows*cols; ++i)
			data[i] = 0;
	else
		throw ("Bad Type");
		
	if (Type == 1 && rows == cols)
		for (unsigned long i=0; i<rows; ++i) 
			data[i*cols + i]=1;
	else if (Type == 1)
		throw ("Not Square");
			
}



Matrix::Matrix(unsigned int Rows, unsigned int Cols, double Data[])
{
	LUData = NULL;

	rows = Rows;
	cols = Cols;
	data = new double[rows*cols];
	memcpy(data, Data, Cols*Rows*sizeof(double));
}



Matrix::Matrix(const Matrix &rhs)
{
	LUData = NULL;

	rows = rhs.rows;
	cols = rhs.cols;
	data = new double[rows*cols];
	memcpy(data, rhs.data, cols*rows*sizeof(double));
}

Matrix::Matrix(const Vector3D &rhs)
{
	LUData = NULL;

	rows = 3;
	cols = 1;
	data = new double[3];
	data[0] = rhs.x;
	data[1] = rhs.y;
	data[2] = rhs.z;
}

Matrix::Matrix(const Point2D &rhs)
{
	LUData = NULL;

	rows = 2;
	cols = 1;
	data = new double[2];
	data[0] = rhs.x;
	data[1] = rhs.y;
}


Matrix::~Matrix()
{
	if (data != NULL) delete [] data;
	if (LUData != NULL) delete [] LUData;

}

double Matrix::GetValue(unsigned int Row, unsigned int Col) const
{
	if (Col >= cols || Row >= rows)
		throw ("Bad index");

	return data[Col + Row*cols];
}


Matrix & Matrix::SetValue(unsigned int Row, unsigned int Col, double NewData) 
{
	if (Col >= cols || Row >= rows)
		throw ("Bad index");
	if (LUData != NULL) 
	{
		delete [] LUData;
		LUData = NULL;
	}
	data[Col + Row*cols] = NewData;
	return *this;
}

Matrix Matrix::GetRow(unsigned int Row) const
{
	if (Row >= rows)
		throw ("Bad index");

	return Matrix(1, cols, &data[Row*cols]);
}


Matrix Matrix::GetCol(unsigned int Col) const
{
	if (Col >= cols)
		throw ("Bad index");
	
	double * ExtRow = new double[rows];
	for (unsigned int i = 0; i < rows; ++i)
	{
		ExtRow[i] = GetValue(i,Col);
	}
	
	Matrix Temp(rows, 1, ExtRow);
	delete [] ExtRow;
	return Temp;
}

Matrix Matrix::RemoveRow(unsigned int Row) const
{
	if (Row >= rows)
		throw ("Bad index");


	Matrix Temp(rows-1, cols);

	for (unsigned int i = 0; i< rows; ++i)
	{
		if (i != Row)
		{
			unsigned int ii = i<Row ? i : i-1;
			for (unsigned int j = 0; j< cols; ++j)
			{
				Temp.data[ii*Temp.cols + j] = data[i*cols + j];
			}
		}
	}

	return Temp;
}

Matrix Matrix::RemoveCol(unsigned int Col) const
{
	if (Col >= cols)
		throw ("Bad index");


	Matrix Temp(rows, cols - 1);

	for (unsigned int i = 0; i< cols; ++i)
	{
		if (i != Col)
		{
			unsigned int ii = i<Col ? i : i-1;
			for (unsigned int j = 0; j< rows; ++j)
			{
				Temp.data[j*Temp.cols + ii] = data[j*cols + i];
			}
		}
	}

	return Temp;
}

Matrix & Matrix::InsertRows(unsigned int Row, const Matrix & rhs)
{
	if (Row > rows)
		throw ("Bad index");
	if (rhs.cols != cols)
		throw ("Bad Matrix Size");
		
	Matrix Temp(rows + rhs.rows, cols);

	for (unsigned int i = 0; i< cols; ++i)
	{
		for (unsigned int j = 0; j< rows + rhs.rows; ++j)
		{
			if (j < Row)
				Temp.SetValue(j,i,GetValue(j,i));
			else if (j >= Row && j < Row + rhs.rows)
				Temp.SetValue(j,i,rhs.GetValue(j-Row,i));
			else
				Temp.SetValue(j,i,GetValue(j-rhs.rows,i));
		}
	}

	return *this = Temp;
}

Matrix & Matrix::InsertCols(unsigned int Col, const Matrix & rhs)
{
	if (Col > cols)
		throw ("Bad index");
	if (rhs.rows != rows)
		throw ("Bad Matrix Size");

	Matrix Temp(rows, cols+rhs.cols);

	for (unsigned int i = 0; i< cols+rhs.cols; ++i)
	{
		for (unsigned int j = 0; j< rows; ++j)
		{
			if (i < Col)
				Temp.SetValue(j,i,GetValue(j,i));
			else if (i >= Col && i < Col + rhs.cols)
				Temp.SetValue(j,i,rhs.GetValue(j,i-Col));
			else
				Temp.SetValue(j,i,GetValue(j,i-rhs.cols));
		}
	}

	return *this = Temp;
}

void Matrix::Resize (int newRows, int newCols)
{
	Matrix temp ((newRows == -1) ? rows : newRows, (newCols == -1) ? cols : newCols, (unsigned int) 0);

	for (unsigned int i = 0; i < rows; i++)
	{
		for (unsigned int j = 0; j < cols; j++)
		{
			temp.SetValue (i, j, data[i * cols + j]);
		}
	}

	*this = temp;
}

void Matrix::Zero (void)
{
	for (unsigned int i = 0; i < rows; i++)
		for (unsigned int j = 0; j < cols; j++)
			data[i * cols + j] = 0.0f;
}





bool Matrix::operator == (const Matrix &rhs) const
{
	if (cols != rhs.cols || rows != rhs.rows)
		return false;
		
	return memcmp(data, rhs.data, cols*rows) == 0;
}

bool Matrix::operator != (const Matrix &rhs) const
{
	return !(*this == rhs);
}

Matrix & Matrix::operator = (const Matrix &rhs)
{
	if (data != NULL)
		delete [] data;
	if (LUData != NULL) 
	{
		delete [] LUData;
		LUData = NULL;
	}

	rows = rhs.rows;
	cols = rhs.cols;
	data = new double[rows*cols];
	memcpy(data, rhs.data, rows * cols*sizeof(double));

	return *this;
}

Matrix Matrix::T(void) const
{
	Matrix Temp(cols,rows,static_cast<unsigned int> (0));
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< cols; ++j)
			Temp.SetValue(j,i,GetValue(i,j));
	return Temp;
}
		
Matrix Matrix::Inv (void)
{
	if (rows != cols)
		throw ("Not Square");
	if (Det() == 0)
		throw ("Singular");

	Matrix Temp(rows,0);
	Matrix I(rows,cols,1);
	// Get L and U
    for (unsigned int i = 0; i < rows; ++i)
	{
		Matrix y = FwdSubL(GetL(), I.GetCol(i));
		Temp.InsertCols(i, BackSubU(GetU(), y));
	}
	return Temp;	

}

double Matrix::Det (bool * Cols, unsigned int Row) const
{
	if (Row == rows - 1)
	{
		// Return the empty Col
		for (unsigned int ii = 0; ii < cols; ++ii)
			if (Cols[ii])
				return data[ii + Row*cols];	
		return 0;
	}
	else
	{
		// Recurse for each free collumn
		double Temp = 0;
		int sign = 1;
		for (unsigned int ii=0;ii<cols;++ii)
		{
			if (Cols[ii])
			{
				Cols[ii] = false;
				Temp += GetValue(Row, ii) * sign * Det(Cols, Row+1);
				Cols[ii] = true;
				sign *= -1;
			}
		}
		return Temp;
	}
}
	

double Matrix::Det (void) const
{
	if (rows != cols)
		throw ("Not Square");
		
	if (rows==1)
		return data[0];

	bool * Cols = new bool[rows];
	for (unsigned int ii = 0; ii < cols; ++ii)
		Cols[ii] = true;
    double Temp = Det(Cols, 0);
	delete [] Cols;
	return Temp;
}
/*double Matrix::Det (void) const
{
	if (rows != cols)
		throw ("Not Square");
		
	if (rows==1)
		return data[0];

	double Temp = 0;
	int sign = 1;
	for (unsigned int ii=0;ii<cols;++ii)
	{
		Temp += GetValue(0, ii) * sign * RemoveCol(ii).RemoveRow(0).Det();
		sign *= -1;
	}
	return Temp;
}*/

Matrix Matrix::MPInv() 
{
	return (T() * (*this)).Inv() * T();
}

void Matrix::LUDecomp()
{
	if (rows != cols)
		throw ("Not Square");

	// Solve using LU factorisation
	Matrix U(*this);
	Matrix L(rows, cols, static_cast<unsigned int> (0));
	
	for (unsigned int CurCol = 0; CurCol < cols-1; ++CurCol)
	{
		// Get L Values for column
		for (unsigned int CurRow = CurCol + 1; CurRow < rows; ++CurRow)
		{
			L.SetValue(CurRow, CurCol, U.GetValue(CurRow, CurCol)/U.GetValue(CurCol, CurCol));
			// Modify Row in U matrix
			for (unsigned int TempCol = 0; TempCol < cols; ++TempCol)
			{
				U.SetValue(CurRow, TempCol, U.GetValue(CurRow, TempCol) - U.GetValue(CurCol, TempCol) * L.GetValue(CurRow, CurCol));
			}
		}
	}

	Matrix Temp(L);
	Temp+= U;

	LUData = new double[rows*cols];
	memcpy(LUData, Temp.data, rows*cols * sizeof(double));

}

Matrix Matrix::GetL(void)
{
	if (LUData == NULL)
		LUDecomp();
	Matrix Temp(cols, rows, LUData);
	for (unsigned int i = 0; i < cols; ++i)
		for (unsigned int j = 0; j <= i; ++j)
			Temp.SetValue(j,i,0);
	return Temp + Matrix(rows, cols, 1);
}

Matrix Matrix::GetU(void)
{
	if (LUData == NULL)
		LUDecomp();
	Matrix Temp(cols, rows, LUData);
	for (unsigned int j = 1; j < rows; ++j)
		for (unsigned int i = 0; i < j; ++i)
			Temp.SetValue(j, i, 0);
	return Temp;
}

double Matrix::Sum(void) const
{
	double Temp = 0;
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< cols; ++j)
			Temp += GetValue(i,j);
	return Temp;
}

double Matrix::Avg(void) const
{
	return Sum() / (rows * cols);
}

double Matrix::Median(void) const
{
	if (rows == 0 || cols == 0)
		throw "Null Matrix, No median";
	// Slow and nasty way, but quick to program
	// Should be fixed one day
	Matrix Copy(*this);
	for (unsigned int i = 0; i < rows*cols/2-1; ++i)
	{
		double CurMax = Copy.Max();
		bool found = false;
		for (unsigned int Row = 0; Row < rows && !found; ++ Row)
		{
			for (unsigned int Col = 0; Col < cols && !found; ++ Col)
			{
				if (Copy.GetValue(Row,Col) == CurMax)
				{
					Copy.SetValue(Row,Col,0);
					found = true;
				}
			}
		}
	}
	return Copy.Max();
}

double Matrix::Min(void) const
{
	double Temp = GetValue(0,0);
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< cols; ++j)
			Temp = GetValue(i,j) > Temp ? Temp : GetValue(i,j);
	return Temp;

}

double Matrix::Max(void) const
{
	double Temp = 0;
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< cols; ++j)
			Temp = GetValue(i,j) < Temp ? Temp : GetValue(i,j);
	return Temp;
}

unsigned int Matrix::MaxIndex(unsigned int Col) const
{
	double Temp = 0;
	unsigned int Index = 0;
	for (unsigned int i = 0; i< rows; ++i)
	{
		Index = GetValue(i,Col) < Temp ? Index : i;
		Temp = GetValue(i,Col) < Temp ? Temp : GetValue(i,Col);
	}
			
	return Index;
}

unsigned int Matrix::MinIndex(unsigned int Col) const
{
	double Temp = GetValue(0,0);
	unsigned int Index = 0;
	for (unsigned int i = 0; i< rows; ++i)
	{
			Index = GetValue(i,Col) > Temp ? Index : i;
			Temp = GetValue(i,Col) > Temp ? Temp : GetValue(i,Col);
	}
	return Index;
}


double Matrix::Var(void) const
{
	double Temp = 0;
	double Av = Avg();
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< cols; ++j)
			Temp += (GetValue(i,j) - Av) * (GetValue(i,j) - Av);
	return Temp / (rows * cols);
}

double Matrix::Std(void) const
{
	return sqrt(Var());
}

Matrix Matrix::SortCol(unsigned int Col)
{
	WhichCol = Col;
	qsort(data,rows,sizeof(double)*cols,CompareRow);
	return * this;
}

Matrix Matrix::SortRow(unsigned int Row)
{
	return T().SortCol(Row).T();
}

const Matrix Matrix::FwdSubL(const Matrix &L, const Matrix & b)
{
	if (L.rows != L.cols || L.rows != b.rows || b.cols != 1)
		throw ("Bad Size");

	Matrix Temp(L.rows, 1);
	Temp.SetValue(0, 0, b.GetValue(0,0)/L.GetValue(0,0));
	for (unsigned int i = 1; i < L.rows; ++i)
	{
		double TempSum = 0;
		for (unsigned int j= 0; j < i; ++j)
			TempSum += L.GetValue(i,j) * Temp.GetValue(j,0);
		Temp.SetValue(i,0,(1/L.GetValue(i,i)) * (b.GetValue(i,0) - TempSum));
	}
	return Temp;
}

const Matrix Matrix::BackSubU(const Matrix &U, const Matrix & y)
{
	if (U.rows != U.cols || U.rows != y.rows || y.cols != 1)
		throw ("Bad Size");

	Matrix Temp(U.rows, 1);
	Temp.SetValue(Temp.rows-1, 0, y.GetValue(y.rows-1, 0)/U.GetValue(U.rows-1,U.cols-1));
	for (int i = U.rows -2; i >= 0; --i)
	{
		double TempSum = 0;
		for (unsigned int j= i+1; j < U.rows; ++j)
			TempSum += U.GetValue(i,j) * Temp.GetValue(j,0);
		Temp.SetValue(i,0,(1/U.GetValue(i,i)) * (y.GetValue(i,0) - TempSum));
	}
	return Temp;
}




Matrix Matrix::Solve(Matrix &A, const Matrix &b)
{
	if (A.rows != b.rows || A.rows < A.cols || b.cols != 1)
		throw ("Bad Size");

	if (A.rows == A.cols)
		return A.Inv() * b;
	return A.MPInv() * b;


}

Matrix Matrix::operator + (const Matrix & rhs) const
{
	if (rows != rhs.rows || cols != rhs.cols)
		throw ("Bad Size");
		
	Matrix Temp(*this);
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< cols; ++j)
			Temp.data[i*cols + j] += rhs.data[i*cols + j];
	return Temp;
}

Matrix & Matrix::operator += (const Matrix & rhs) 
{
	return *this = *this + rhs;
}

Matrix Matrix::operator - (const Matrix & rhs) const
{
	if (rows != rhs.rows || cols != rhs.cols)
		throw ("Bad Size");
		
	Matrix Temp(*this);
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< cols; ++j)
			Temp.data[i*cols + j] -= rhs.data[i*cols + j];
	return Temp;
}

Matrix & Matrix::operator -= (const Matrix & rhs) 
{
	return *this = *this - rhs;
}

Matrix Matrix::operator * (const Matrix &rhs) const
{
	if (cols != rhs.rows)
		throw ("Bad Size");
	
	Matrix Temp(rows, rhs.cols, static_cast<unsigned int> (0));
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< rhs.cols; ++j)
			for (unsigned int k =0; k < cols; ++k)
				Temp.data[i*rhs.cols + j] += GetValue(i,k) * rhs.GetValue(k,j);
	return Temp;
}


Matrix Matrix::operator * (const double rhs) const
{
	Matrix Temp(*this);
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< cols; ++j)
			Temp.data[i*cols + j] *= rhs;
	return Temp;
}

Matrix & Matrix::operator *= (const Matrix & rhs) 
{
	return *this = *this * rhs;
}

Matrix & Matrix::operator *= (double rhs) 
{
	return *this = *this * rhs;
}

double& Matrix::operator () (unsigned int Row, unsigned int Col)
{
	if (Col >= cols || Row >= rows)
		throw ("Bad index");

	return data[Col + Row*cols];
}


Matrix Matrix::ScalarProduct(const Matrix & rhs)
{
	if (rows != rhs.rows || cols != rhs.cols)
		throw ("Bad Size");
		
	Matrix Temp(*this);
	for (unsigned int i = 0; i< rows; ++i)
		for (unsigned int j = 0; j< cols; ++j)
			Temp.data[i*cols + j] *= rhs.data[i*cols + j];
	return Temp;	

}

ostream & operator << (ostream & rhs, const Matrix & data) 
{
	cout << "[";
	for (unsigned int i = 0; i< data.rows; ++i)
	{
		for (unsigned int j = 0; j< data.cols; ++j)
			rhs << data.data[i*data.cols + j] << ((j == data.cols-1) ? ";" : " , ");
		rhs << ((i == data.rows-1) ? "]" : "") << endl;
	}
	return rhs;
}
			
	
Vector3D Matrix::ToVector()
{
	if (rows < 3 || cols < 1)
		throw "Bad Size";
	return Vector3D(data[0],data[1],data[2]);

}

Point2D Matrix::ToPoint()
{
	if (rows < 2 || cols < 1)
		throw "Bad Size";
	return Point2D(data[0],data[1]);

}

//////////////////////////////////////////////////////////////////////////////
//	The following block of code is code from the TMatrixD class
//	found at http://cholm.home.cern.ch/cholm/root/principal/
//	This code is based on algorithms from Numerical Recipes in C,
//	available at http://www.nr.com
//	It has been modified to suit our needs and datatypes.
//////////////////////////////////////////////////////////////////////////////

#define ABS(a)			(a >= 0) ? a : -a
#define SIGN(a,b)		(b >= 0) ? ABS(a) : -ABS(a)

Matrix Matrix::EigenVectors (Matrix &eigenValues)
{
	Matrix eigenVectors (*this);
	cout << "Making eigenvectors for " << *this;
	if (IsSymmetric ())
	{
		eigenValues.Resize (rows, 1);
		Matrix offDiag (rows, 1);

//		cout << "Normalised covariance matrix = " << endl << *this << endl;

		// Tridiagonalize the matrix
		MakeTridiagonal (eigenVectors, eigenValues, offDiag);

//		cout << "------------------------" << endl << "MakeTridiagonal:" << endl;
//		cout << "eigenVectors = " << endl << eigenVectors;
//		cout << "eigenValues = " << endl << eigenValues;
//		cout << "offDiag = " << endl << offDiag;

		// Make the eigenvectors and -values
		MakeEigenVectors (eigenValues, offDiag, eigenVectors);

		cout << "------------------------" << endl << "MakeEigenVectors:" << endl;
		cout << "eigenVectors = " << endl << eigenVectors;
		cout << "eigenValues = " << endl << eigenValues;
		cout << "offDiag = " << endl << offDiag;

		// Order the eigenvalues and -vectors
//		EigenSort (eigenVectors, eigenValues);

//		cout << "------------------------" << endl << "EigenSort:" << endl;
//		cout << "eigenVectors = " << endl << eigenVectors;
//		cout << "eigenValues = " << endl << eigenValues;
//		cout << "offDiag = " << endl << offDiag;
//		cout << "------------------------" << endl;

		// Return the result
		return eigenVectors;
	}
	else
	{
		cout << "EigenVectors: Not yet implemented for non-symmetric matrix:" << endl;
		cout << *this;
		return *this;
	}
}

bool Matrix::IsSymmetric (void)
{
	// Check the matrix is square
	if (rows != cols)
	{
		printf ("IsSymmetric: matrix is not square\n");
		return false;
	}

/*	if (fRowLwb != fColLwb)
	{
		Error("IsSymmetric", "row and column start at different values");
		return 0;
	}*/

	// Loop through the data in the matrix, making sure that every
	// element (i, j) == element (j, i).
	unsigned int irow;
	for (irow = 0; irow < rows; irow++)
	{
		unsigned int icol;
		for (icol = 0; icol < irow; icol++)
		{
			if (data[irow * rows + icol] != data[icol * rows + irow])
			{
				return false;
			}
		}
	}
	return true;
}

//	Tridiagonalise the covariance matrix according to the Householder
//	method as described in Numerical Recipes in C (http://www.nr.com) section 11.2.
//	The basic idea is to perform P-2 orthogonal transformation, where each
//	transformation eat away the off-diagonal elements, except the inner most.
void MakeTridiagonal (Matrix &a, Matrix &d, Matrix &e)
{
	const int n = a.Rows ();

/*	if (!a.IsValid()) {
		gROOT->Error("Maketridiagonal", "matrix not initialized");
		return;
	}*/

	if (a.Rows () != a.Cols ())
	{
		printf ("Maketridiagonal: matrix to tridiagonalize must be square\n");
		return;
	}

	if (!a.IsSymmetric ()) {
		printf ("MakeTridiagonal: Can only tridiagonalise symmetric matrix\n");
		a.Zero ();
		d.Zero ();
		e.Zero ();
		return;
	}

	double *pa = a.data;
	double *pd = d.data;
	double *pe = e.data;

	int i;
	for (i = n-1; i > 0; i--)
	{
		const int l = i-1;
		double h = 0;
		double scale = 0;

		if (l > 0)
		{
			for (int k = 0; k <= l; k++)
				scale += ABS (pa[i+k*n]);

			if (scale == 0)
				// Skip transformation
				pe[i] = pa[i+l*n];

			else
			{
				int k;
				for (k = 0; k <= l; k++)
				{
					// Use scaled elements of a for transformation
					pa[i+k*n] /= scale;
					// Calculate sigma in h
					h += pa[i+k*n]*pa[i+k*n];
				}

				double f =  pa[i+l*n];
				double g =  (f >= 0. ? -sqrt(h) : sqrt(h));
				pe[i] =  scale*g;
				h -= f*g;				// Now h is eq. (11.2.4) in "Numerical ..."
				pa[i+l*n] = f-g;
				f = 0;

				int j;
				for (j = 0; j <= l; j++)
				{
					// Store the u/H in ith column of a;
					pa[j+i*n] = pa[i+j*n]/h;
					// Form element A dot u in g;
					g = 0;

					int k;
					for (k = 0; k <= j; k++)
						g += pa[j+k*n]*pa[i+k*n];

					for (k = j+1; k <= l; k++)
						g += pa[k+j*n]*pa[i+k*n];

					// Form element of vector p in temporarily unused element of
					// e
					pe[j] =  g/h;
					f    += pe[j]*pa[i+j*n];
				}
				// Form K eq (11.2.11)
				const double hh = f/(h+h);

				// Form vector q and store in e overwriting p
				for (j = 0; j <= l; j++) 
				{
					f = pa[i+j*n];
					pe[j] = g = pe[j]-hh*f;

					int k;
					for (k = 0; k <= j; k++)
						// Reduce a, eq (11.2.13)
						pa[j+k*n] -= (f*pe[k]+g*pa[i+k*n]);
				}
			}
		}
		else
			pe[i] = pa[i+l*n];

		pd[i] = h;
	}

	pd[0] = 0;
	pe[0] = 0;

	for (i = 0; i < n; i++)
	{
		// Begin accumulation of transformation matrix
		const int l = i-1;

		if (pd[i])
		{
			// This block is skipped if i = 0;
			int j;
			for (j = 0; j <= l; j++)
			{
				double g = 0;

				int k;
				for (k = 0; k <= l; k++)
					// Use vector u/H stored in a to form P dot Q
					g += pa[i+k*n]*pa[k+j*n];

				for (k = 0; k <= l; k++)
					pa[k+j*n] -= g*pa[k+i*n];
			}
		}

		pd[i] = pa[i+i*n];
		pa[i+i*n] = 1;

		int j;
		for (j = 0; j <= l; j++)
		{
			pa[j+i*n] = pa[i+j*n] = 0;
		}
	}
}

//	Find eigenvalues and vectors of tridiagonalised covariance matrix according 
//	to the QL with implicit shift algorithm from Numerical Recipes in C section 11.3.
//	The basic idea is to find matrices Q and L so that C=Q.L, where is Q orthogonal 
//	and L is lower triangular.
//	d = eigenValues, e = offDiag, z = eigenVectors
void MakeEigenVectors (Matrix &d, Matrix &e, Matrix &z)
{
	const int n = z.rows;

	double *pd = d.data;
	double *pe = e.data;
	double *pz = z.data;

	// It's convenient to renumber the e vector elements
	int l;
	for (l = 1; l < n; l++)
		pe[l-1] = pe[l];
	pe[n-1] = 0;

	for (l = 0; l < n; l++)
	{
		int iter = 0;
		int m = 0;

		do
		{
			for (m = l; m < n-1; m++)
			{
				// Look for a single small sub-diagonal element  to split the matrix
				double dd = ABS(pd[m])+ ABS(pd[m+1]);
				if ((double)(ABS(pe[m])+dd) == dd)
					break;
			}

			if (m != l)
			{
				if (iter++ == 30)
				{
					printf ("MakeEigenVectors: too many iterations\n");
					return;
				}

				// Form shift
				double g = (pd[l+1]-pd[l])/(2.0f*pe[l]);
				double r = sqrt((g*g)+1);
				// This is d_m-k_s
				g = pd[m]-pd[l]+pe[l]/(g + SIGN(r,g));
				double s = 1;
				double c = 1;
				double p = 0;
				int i = 0;
				for (i = m-1; i >= l; i--)
				{
					// A plane rotation as in the original QL, followed by
					// Givens rotations to restore tridiagonal form
					double f = s*pe[i];
					const double b = c*pe[i];
					r = sqrt((f*f)+(g*g));
					pe[i+1] = r;

					if (r == 0)
					{
						// Recover from underflow
						pd[i+1] -= p;
						pe[m]   =  0;
						break;
					}
					s = f/r;
					c = g/r;
					g = pd[i+1]-p;
					r = (pd[i]-g)*s+2*c*b;
					p = s*r;
					pd[i+1] = g+p;
					g = c*r-b;

					int k;
					for (k = 0; k < n; k++)
					{
						// Form Eigenvectors
						f = pz[k+(i+1)*n];
						pz[k+(i+1)*n] = s*pz[k+i*n]+c*f;
						pz[k+i*n] = c*pz[k+i*n]-s*f;
					}
				}  // for (i = m)

				if (r == 0 && i >= l)
					continue;

				pd[l] -= p;
				pe[l] = g;
				pe[m] = 0;

			} // if (m != l)
		}
		while (m != l);
	} // for (l = 0)
}

//	Orders the eigenvalues and eigenvectors by ascending eigenvalue.
//	This algorithm is from Numerical Recipes in C section 11.1.
void EigenSort (Matrix &eigenVectors, Matrix &eigenValues)
{
	int n = eigenVectors.rows;

	double *pVec = eigenVectors.data;
	double *pVal = eigenValues.data;

	int i;
	for (i = 0; i < n; i++)
	{
		int k = i;
		double p = pVal[i];

		int j;
		for (j = i + 1; j < n; j++)
			if (pVal[j] >= p)
			{
				k = j;
				p = pVal[j];
			}

			if (k != i)
			{
				pVal[k] = pVal[i];
				pVal[i] = p;

				for (j = 0; j < n; j++)
				{
					p = pVec[j+i*n];
					pVec[j+i*n] = pVec[j+k*n];
					pVec[j+k*n] = p;
				}
			}
	}
}

int Matrix::CompareRow(void const * Left, void const * Right)
{
	double LeftVal = reinterpret_cast<const double *> (Left)[WhichCol];
	double RightVal = reinterpret_cast<const double *> (Right)[WhichCol];
	
	if (LeftVal < RightVal)
		return -1;
	else if (LeftVal > RightVal)
		return 1;
	return 0;	
}

//////////////////////////////////////////////////////////////////////////////
