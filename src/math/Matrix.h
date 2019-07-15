//////////////////////////////////////////////////////////////////////
// Matrix.h
//
//	Interfaces declaration of CMatrix for matrix operation
//	Modified by Tang Qing
//	Date: Oct 19, 2009
//
//	modified the dll preprocesser
//	Modified by YichaoSun
//	Date: Dec 25, 2009

//////////////////////////////////////////////////////////////////////
#ifndef _CMATRIX_
#define _CMATRIX_

#include "string.h"
#include "arm_math.h"

class CMatrix
{

public:

    //
    // Constructor and destructor
    //

    CMatrix();													// 基础构造函数
    CMatrix(int nRows, int nCols);					// 指定行列构造函数
    CMatrix(int nRows, int nCols,const float value[]);	// 指定数据构造函数
		CMatrix(int nRows, int nCols,float* pData);	//Constructor for CMSIS
    CMatrix(int nSize);								// 方阵构造函数
    CMatrix(int nSize, const float value[]);				// 指定数据方阵构造函数
    CMatrix(const CMatrix& other);					// 拷贝构造函数
    bool	Init(int nRows, int nCols);				// 初始化矩阵
    bool	MakeUnitMatrix(int nSize);				// 将方阵初始化为单位矩阵
		bool	MakeUnitMatrix();				// 将方阵初始化为单位矩阵
    virtual ~CMatrix();								// 析构函数

    // Output and display
    bool print(void);

    //
    // operation of elements and value
    //

    bool	SetElement(int nRow, int nCol,float value);	// 设置指定元素的值
    float	GetElement(int nRow, int nCol) const;			// 获取指定元素的值
    void    SetData(const float value[]);						// 设置矩阵的值
    void	SetDataF(float value[], int nRows, int nCols);	// 设置矩阵的值（调用Fortran专用。慎用！）
    void    SetDataVertical(float value[]);						// 设置矩阵的值竖直方向
    int		GetNumColumns() const;							// 获取矩阵的列数
    int		GetNumRows() const;								// 获取矩阵的行数
//    int     GetRowVector(int nRow, float* pVector) const;	// 获取矩阵的指定行矩阵
//    int     GetColVector(int nCol, float* pVector) const;	// 获取矩阵的指定列矩阵
    float* GetData() const;								// 获取矩阵的值

    //
    // mathematical operation
    //

    CMatrix& operator=(const CMatrix& other);
    bool operator==(const CMatrix& other) const;
    bool operator!=(const CMatrix& other) const;
    CMatrix	operator|(const CMatrix& other) const;		// Column Add
    CMatrix	operator&(const CMatrix& other) const;		// Row Add
    CMatrix	operator+(const CMatrix& other) const;
    CMatrix	operator-(const CMatrix& other) const;
    CMatrix	operator*(float value) const;
    CMatrix	operator*(const CMatrix& other) const;
    CMatrix	operator/(float value) const;
    // 复矩阵乘法
    bool CMul(const CMatrix& AR, const CMatrix& AI, const CMatrix& BR, const CMatrix& BI, CMatrix& CR, CMatrix& CI) const;
    // 矩阵的转置
    CMatrix Transpose() const;

    float operator[](int index) const;

    //
    // 算法
    //

    // 实矩阵求逆的全选主元高斯－约当法
    bool InvertGaussJordan();


    // Get the sub matrix from the parents' matrix
    CMatrix GetSubMatrix(int row1, int row2, int col1, int col2);

    float norm(void) const;


    CMatrix GetColVectorMat(int nCol);


    //
    // 保护性数据成员
    //
protected:
		arm_matrix_instance_f32 MatIns_;

    //
    // 内部函数
    //
private:
    void ppp(float a[], float e[], float s[], float v[], int m, int n);
    void sss(float fg[2], float cs[2]);


};


#endif
