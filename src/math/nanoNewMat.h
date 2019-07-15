#ifndef _NANO_NEWMAT_
#define _NANO_NEWMAT_

#include "Matrix.h"

class CColumnVector:
	private CMatrix
{
	public:
		CColumnVector();
		CColumnVector(int nRow):CMatrix(nRow, 1){};
		CColumnVector(int nRow, float* pData):CMatrix(nRow, 1, pData){};
		CColumnVector(const CMatrix& other):CMatrix(other){};
			
		bool print(void){return CMatrix::print();}
		
		//
    // operation of elements and value
    //

    bool	SetElement(int nRow, float value) {return CMatrix::SetElement(nRow, 1, value);}
    float	GetElement(int nRow) const {return CMatrix::GetElement(nRow, 1);}
    int		GetNumRows() const {return CMatrix::GetNumRows();}
		
		//
    // mathematical operation
    //
		
		CMatrix& operator=(const CMatrix& other) {return CMatrix::operator=(other);}
    bool operator==(const CMatrix& other) const {return CMatrix::operator==(other);}
    bool operator!=(const CMatrix& other) const	{return CMatrix::operator!=(other);}
    CMatrix	operator+(const CColumnVector& other) const {return CMatrix::operator+(other);}
    CMatrix	operator-(const CColumnVector& other) const {return CMatrix::operator-(other);}
    CMatrix	operator*(float value) const {return CMatrix::operator*(value);}
    CMatrix	operator*(const CMatrix& other) const {return CMatrix::operator*(other);}
    CMatrix	operator/(float value) const {return CMatrix::operator/(value);}
		
		CMatrix Transpose() const {return CMatrix::Transpose();}
		
		float operator[](int index) const {return CMatrix::operator[](index);}
		
};

#endif
//end of file
