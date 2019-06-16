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
#ifndef _MATRIX_H
#define _MATRIX_H

#include <iostream>
#include "vector3d.h"
#include "geometry.h"

#ifdef Max
#undef Max
#endif

#ifdef Min
#undef Min
#endif


using namespace std;

class Matrix
{
public:
	Matrix(unsigned int Rows=0, unsigned int Cols=0);
	Matrix(unsigned int Rows, unsigned int Cols, unsigned int Type);
	Matrix(unsigned int Rows, unsigned int Cols, double Data[]);
	Matrix(const Matrix &rhs);
	Matrix(const Vector3D &rhs);
	Matrix(const Point2D &rhs);

	~Matrix();

	double GetValue(unsigned int Row, unsigned int Col) const;
	Matrix & SetValue(unsigned int Row, unsigned int Col, double data);
	Matrix GetRow(unsigned int Row) const;
	Matrix GetCol(unsigned int Col) const;
	Matrix RemoveRow(unsigned int Row) const;
	Matrix RemoveCol(unsigned int Col) const;
	Matrix & InsertRows(unsigned int Row, const Matrix & rhs);
	Matrix & InsertCols(unsigned int Col, const Matrix & rhs);
	void Resize (int newRows, int newCols);
	void Zero (void);

	unsigned int Rows() const {return rows;} ;
	unsigned int Cols() const {return cols;} ;
	

	bool operator == (const Matrix &rhs) const;
	bool operator != (const Matrix &rhs) const;
	
	Matrix & operator = (const Matrix &rhs);
	Matrix T (void) const;
	Matrix Inv (void);
	double Det (void) const;
	Matrix MPInv(void);
	Matrix GetL(void);
	Matrix GetU(void);

	double Sum(void) const;
	double Avg(void) const;
	double Median(void) const;
	double Min(void) const;
	double Max(void) const;
	unsigned int MaxIndex(unsigned int Col = 0) const;
	unsigned int MinIndex(unsigned int Col = 0) const;
	double Var(void) const;
	double Std(void) const;

	Matrix SortCol(unsigned int Col = 0);	Matrix SortRow(unsigned int Row = 0);
	static const Matrix FwdSubL(const Matrix &A, const Matrix & b);
	static const Matrix BackSubU(const Matrix &A, const Matrix & b);
	static Matrix Solve(Matrix &A, const Matrix &b);

	Matrix operator + (const Matrix &rhs) const;
	Matrix & operator += (const Matrix &rhs);
	Matrix operator - (const Matrix &rhs) const;
	Matrix & operator -= (const Matrix &rhs);
	Matrix operator * (const Matrix &rhs) const;
	Matrix operator * (const double rhs) const;
	Matrix & operator *= (const Matrix &rhs);
	Matrix & operator *= (const double rhs);
	double& operator () (unsigned int Row, unsigned int Col = 0);

	Matrix ScalarProduct(const Matrix & rhs);

	Vector3D ToVector();
	Point2D ToPoint();

	
	friend ostream &operator << (ostream & rhs, const Matrix & data);

	//	Eigenvector functions
	Matrix EigenVectors (Matrix &eigenValues);
	bool IsSymmetric (void);
	friend void MakeTridiagonal (Matrix &a, Matrix &d, Matrix &e);
	friend void MakeEigenVectors (Matrix &d, Matrix &e, Matrix &z);
	friend void EigenSort (Matrix &eigenVectors, Matrix &eigenValues);


private:
	double Det (bool * Cols, unsigned int Row) const;

	unsigned int rows;
	unsigned int cols;
	double * data;

	double * LUData;
	void LUDecomp();

	// helpers for quicksort
	static unsigned int WhichCol;
	static int CompareRow(const void * Left, const void * Right);

};



ostream &operator << (ostream & rhs, const Matrix & data);


#endif
