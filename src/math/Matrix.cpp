//////////////////////////////////////////////////////////////////////
// Matrix.cpp
//
// Modified by YichaoSun for portability
// achieve file of CMatrix for matrix operation

//  modified the dll preprocesser
//	Modified by YichaoSun
//	Date: Dec 25, 2009
//////////////////////////////////////////////////////////////////////

#include "Matrix.h"
#include "Console.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// basic constructor
//////////////////////////////////////////////////////////////////////
CMatrix::CMatrix()
{
	MatIns_.numCols = 1;
	MatIns_.numRows = 1;
	MatIns_.pData = NULL;
  bool bSuccess = Init(MatIns_.numRows, MatIns_.numCols );
}

//////////////////////////////////////////////////////////////////////
// 指定行列构造函数
//
// params:
// 1. int nRows - 指定的matrix行数
// 2. int nCols - 指定的matrix列数
//////////////////////////////////////////////////////////////////////
CMatrix::CMatrix(int nRows, int nCols)
{
    MatIns_.numRows = nRows;
    MatIns_.numCols = nCols;
    MatIns_.pData = NULL;
    bool bSuccess = Init(MatIns_.numRows, MatIns_.numCols);

}

//////////////////////////////////////////////////////////////////////
// 指定值构造函数
//
// params:
// 1. int nRows - 指定的matrix行数
// 2. int nCols - 指定的matrix列数
// 3. double value[] - 一维数组，长度为nRows*nCols，存储matrix各元素的值
//////////////////////////////////////////////////////////////////////
CMatrix::CMatrix(int nRows, int nCols,const float value[])
{
    MatIns_.numRows = nRows;
    MatIns_.numCols = nCols;
    MatIns_.pData = NULL;
    bool bSuccess = Init(MatIns_.numRows, MatIns_.numCols);


    SetData(value);
}

//////////////////////////////////////////////////////////////////////
// 方阵构造函数
//
// params:
// 1. int nSize - 方阵行列数
//////////////////////////////////////////////////////////////////////
CMatrix::CMatrix(int nSize)
{
    MatIns_.numRows = nSize;
    MatIns_.numCols = nSize;
    MatIns_.pData = NULL;
    bool bSuccess = Init(nSize, nSize);

}

//////////////////////////////////////////////////////////////////////
// 方阵构造函数
//
// params:
// 1. int nSize - 方阵行列数
// 2. double value[] - 一维数组，长度为nRows*nRows，存储方阵各元素的值
//////////////////////////////////////////////////////////////////////
CMatrix::CMatrix(int nSize,const float value[])
{
    MatIns_.numRows = nSize;
    MatIns_.numCols = nSize;
    MatIns_.pData = NULL;
    bool bSuccess = Init(nSize, nSize);


    SetData(value);
}

//////////////////////////////////////////////////////////////////////
// 拷贝构造函数
//
// params:
// 1. const CMatrix& other - 源matrix
//////////////////////////////////////////////////////////////////////
CMatrix::CMatrix(const CMatrix& other)
{
    MatIns_.numCols = other.GetNumColumns();
    MatIns_.numRows = other.GetNumRows();
    MatIns_.pData = NULL;
    bool bSuccess = Init(MatIns_.numRows, MatIns_.numCols);


    // copy the pointer
    memcpy(MatIns_.pData, other.MatIns_.pData, sizeof(float)*MatIns_.numCols*MatIns_.numRows);
}

//////////////////////////////////////////////////////////////////////
// Constructor for CMSIS matrix operation
// Created at 2017/07/21 by chenxx
//////////////////////////////////////////////////////////////////////
CMatrix::CMatrix(int nRows, int nCols,float* pData)
{
	arm_mat_init_f32(&MatIns_, nRows, nCols, pData);
}

//////////////////////////////////////////////////////////////////////
CMatrix::~CMatrix()
{
    if (MatIns_.pData)
    {
        delete[] MatIns_.pData;
        MatIns_.pData = NULL;
    }
}

//////////////////////////////////////////////////////////////////////
// 初始化函数
//
// params:
// 1. int nRows - 指定的matrix行数
// 2. int nCols - 指定的matrix列数
//
// return: bool 型，初始化是否成功
//////////////////////////////////////////////////////////////////////
bool CMatrix::Init(int nRows, int nCols)
{
    if (MatIns_.pData)
    {
        delete[] MatIns_.pData;
        MatIns_.pData = NULL;
    }

    MatIns_.numRows = nRows;
    MatIns_.numCols = nCols;
    int nSize = nCols*nRows;
    if (nSize < 0)
        return false;

    // allocate memory
    MatIns_.pData = new float[nSize];

    if (MatIns_.pData == NULL)
        return false;					// memory allocate failed

    // quoted for better portability on Linux
    //if (IsBadReadPtr(MatIns_.pData, sizeof(float) * nSize))
    //	return false;

    // set all elements to 0
    memset(MatIns_.pData, 0, sizeof(float) * nSize);

    return true;
}

//////////////////////////////////////////////////////////////////////
// 将方阵初始化为单位matrix
//
// params:
// 1. int nSize - 方阵行列数
//
// return: bool 型，初始化是否成功
//////////////////////////////////////////////////////////////////////
bool CMatrix::MakeUnitMatrix(int nSize)
{
    if (! Init(nSize, nSize))
        return false;

    for (int i=0; i<nSize; ++i)
        for (int j=0; j<nSize; ++j)
            if (i == j)
                SetElement(i, j, 1);

    return true;
}

bool CMatrix::MakeUnitMatrix()
{
	if(MatIns_.numCols != MatIns_.numRows)
		return false;
	
	for (int i=0; i < MatIns_.numCols; ++i)
        for (int j=0; j < MatIns_.numCols; ++j)
            if (i == j)
                SetElement(i, j, 1);

    return true;
}


//////////////////////////////////////////////////////////////////////
// 设置matrix各元素的值
//
// params:
// 1. double value[] - 一维数组，长度为m_nNumColumns*m_nNumRows，存储
//                     matrix各元素的值
//
// return: 无
//////////////////////////////////////////////////////////////////////
void CMatrix::SetData(const float value[])
{
    // empty the memory
    memset(MatIns_.pData, 0, sizeof(float) * MatIns_.numCols*MatIns_.numRows);
    // copy data
    memcpy(MatIns_.pData, value, sizeof(float)*MatIns_.numCols*MatIns_.numRows);
}
//////////////////////////////////////////////////////////////////////
// 设置matrix指定分块的值(调用Fortran专用)
//
// params:
// 1. double value[] - 一维数组，长度为m_nNumColumns*m_nNumRows，存储
//                     matrix各元素的值
// 2. int nRows - 需要覆盖的matrix的行数
// 3. int nCols - 需要覆盖的matrix的列数
// return: 无
//////////////////////////////////////////////////////////////////////
void CMatrix::SetDataF(float value[], int nRows, int nCols)
{
    // empty the memory
    memset(MatIns_.pData, 0, sizeof(float)*MatIns_.numCols*MatIns_.numRows);
    // copy data
    memcpy(MatIns_.pData, value, sizeof(float)*nRows*nCols);
}



//////////////////////////////////////////////////////////////////////
// 设置matrix各元素的值
//
// params:
// 1. double value[] - 一维数组，长度为m_nNumColumns*m_nNumRows，存储
//                     matrix各元素的值
//
// return: 无
//////////////////////////////////////////////////////////////////////
void CMatrix::SetDataVertical(float value[])
{
    // empty the memory
    memset(MatIns_.pData, 0, sizeof(float) * MatIns_.numCols*MatIns_.numRows);
    // copy data
    //memcpy(MatIns_.pData, value, sizeof(float)*MatIns_.numCols*MatIns_.numRows);
    for(int i = 0; i < MatIns_.numRows; i++){
        for(int j = 0; j < MatIns_.numCols; j++){
            MatIns_.pData[i*MatIns_.numCols+j] = value[i + j*MatIns_.numRows];
        }
    }
}


//////////////////////////////////////////////////////////////////////
// 设置指定元素的值
//
// params:
// 1. int nRows - 指定的matrix行数
// 2. int nCols - 指定的matrix列数
// 3. double value - 指定元素的值
//
// return: bool 型，说明设置是否成功
//////////////////////////////////////////////////////////////////////
bool CMatrix::SetElement(int nRow, int nCol,float value)
{
    if (nCol < 0 || nCol >= MatIns_.numCols || nRow < 0 || nRow >= MatIns_.numRows)
        return false;						// array bounds error
    if (MatIns_.pData == NULL)
        return false;							// bad pointer error

    MatIns_.pData[nCol + nRow * MatIns_.numCols] = value;

    return true;
}

//////////////////////////////////////////////////////////////////////
// 设置指定元素的值
//
// params:
// 1. int nRows - 指定的matrix行数
// 2. int nCols - 指定的matrix列数
//
// return: double 型，指定元素的值
//////////////////////////////////////////////////////////////////////
float CMatrix::GetElement(int nRow, int nCol) const
{
    //ASSERT(nCol >= 0 && nCol < MatIns_.numCols && nRow >= 0 && nRow < MatIns_.numRows); // array bounds error
    //ASSERT(MatIns_.pData);							// bad pointer error
    return MatIns_.pData[nCol + nRow * MatIns_.numCols] ;
}

//////////////////////////////////////////////////////////////////////
// 获取matrix的列数
//
// params:无
//
// return: int 型，matrix的列数
//////////////////////////////////////////////////////////////////////
int	CMatrix::GetNumColumns() const
{
    return MatIns_.numCols;
}

//////////////////////////////////////////////////////////////////////
// 获取matrix的行数
//
// params:无
//
// return: int 型，matrix的行数
//////////////////////////////////////////////////////////////////////
int	CMatrix::GetNumRows() const
{
    return MatIns_.numRows;
}

//////////////////////////////////////////////////////////////////////
// 获取matrix的数据
//
// params:无
//
// return: double型指针，指向matrix各元素的数据缓冲区
//////////////////////////////////////////////////////////////////////
float* CMatrix::GetData() const
{
    return MatIns_.pData;
}

//////////////////////////////////////////////////////////////////////
// 重载运算符=，给matrix赋值
//
// params:
// 1. const CMatrix& other - 用于给matrix赋值的源matrix
//
// return: CMatrix型的引用，所引用的matrix与other相等
//////////////////////////////////////////////////////////////////////
CMatrix& CMatrix::operator=(const CMatrix& other)
{
    if (&other != this)
    {
        bool bSuccess = Init(other.GetNumRows(), other.GetNumColumns());
        //ASSERT(bSuccess);

        // copy the pointer
        memcpy(MatIns_.pData, other.MatIns_.pData, sizeof(float)*MatIns_.numCols*MatIns_.numRows);
    }

    // finally return a reference to ourselves
    return *this ;
}


//////////////////////////////////////////////////////////////////////
// 重载运算符==，判断matrix是否相等
//
// params:
// 1. const CMatrix& other - 用于比较的matrix
//
// return: bool 型，两个matrix相等则为true，否则为false
//////////////////////////////////////////////////////////////////////
bool CMatrix::operator==(const CMatrix& other) const
{
    // 首先检查行列数是否相等
    if (MatIns_.numCols != other.GetNumColumns() || MatIns_.numRows != other.GetNumRows())
        return false;

    for (int i=0; i<MatIns_.numRows; ++i)
    {
        for (int j=0; j<MatIns_.numCols; ++j)
        {
            if (GetElement(i, j) != other.GetElement(i, j))
                return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// 重载运算符!=，判断matrix是否不相等
//
// params:
// 1. const CMatrix& other - 用于比较的matrix
//
// return: bool 型，两个不matrix相等则为true，否则为false
//////////////////////////////////////////////////////////////////////
bool CMatrix::operator!=(const CMatrix& other) const
{
    return !(*this == other);
}

//////////////////////////////////////////////////////////////////////
// 重载运算符+，实现matrix的加法
//
// params:
// 1. const CMatrix& other - 与指定matrix相加的matrix
//
// return: CMatrix型，指定matrix与other相加之和
//////////////////////////////////////////////////////////////////////
CMatrix	CMatrix::operator+(const CMatrix& other) const
{
    // 首先检查行列数是否相等
    //ASSERT (MatIns_.numCols == other.GetNumColumns() && MatIns_.numRows == other.GetNumRows());

    // 构造结果matrix
    CMatrix	result(*this) ;		// 拷贝构造
    // matrix加法
		arm_mat_add_f32(&MatIns_, &other.MatIns_, &result.MatIns_);

    return result ;
}

//////////////////////////////////////////////////////////////////////
// 重载运算符|，实现matrix的列拼接
//
// params:
// 1. const CMatrix& other - 与指定matrix相拼接的matrix
//
// return: CMatrix型，指定matrix与other相拼加之结果
//////////////////////////////////////////////////////////////////////
CMatrix	CMatrix::operator|(const CMatrix& other) const
{
    // 首先检查行列数是否相等
    //ASSERT (MatIns_.numCols == other.GetNumColumns() && MatIns_.numRows == other.GetNumRows());
    if(MatIns_.numRows != other.GetNumRows()){
			Console::Instance()->printf("Matrix Dimention don't match!\r\n");
    }

    // 构造结果matrix
    int row = MatIns_.numRows;
    int col = MatIns_.numCols + other.GetNumColumns();
    CMatrix result(row, col);


    // matrix加法
    for (int i = 0 ; i < row ; ++i)
    {
        for (int j = 0 ; j < col; ++j)
            if(j < MatIns_.numCols)
                result.SetElement(i, j, GetElement(i,j)) ;
            else
                result.SetElement(i, j, other.GetElement(i, j-MatIns_.numCols));
    }
    return result ;
}

//////////////////////////////////////////////////////////////////////
// 重载运算符_，实现matrix的列拼接
//
// params:
// 1. const CMatrix& other - 与指定matrix相拼接的matrix
//
// return: CMatrix型，指定matrix与other相拼加之结果
//////////////////////////////////////////////////////////////////////
CMatrix	CMatrix::operator&(const CMatrix& other) const
{
    // 首先检查行列数是否相等
    //ASSERT (MatIns_.numCols == other.GetNumColumns() && MatIns_.numRows == other.GetNumRows());
    if(MatIns_.numCols != other.GetNumColumns()){
			Console::Instance()->printf("Matrix Dimention don't match! Columns\r\n");
    }

    // 构造结果matrix
    int row = MatIns_.numRows + other.GetNumRows();
    int col = MatIns_.numCols;
    CMatrix result(row, col);


    for (int i = 0 ; i < row; ++i)
    {
        for (int j = 0 ; j < col; ++j)
            if(i < MatIns_.numRows)
                result.SetElement(i, j, GetElement(i,j)) ;
            else
                result.SetElement(i, j, other.GetElement(i-MatIns_.numRows, j));
    }
    return result ;
}

//////////////////////////////////////////////////////////////////////
// 重载运算符-，实现matrix的减法
//
// params:
// 1. const CMatrix& other - 与指定matrix相减的matrix
//
// return: CMatrix型，指定matrix与other相减之差
//////////////////////////////////////////////////////////////////////
CMatrix	CMatrix::operator-(const CMatrix& other) const
{
    // 首先检查行列数是否相等
    //ASSERT (MatIns_.numCols == other.GetNumColumns() && MatIns_.numRows == other.GetNumRows());

    // 构造目标matrix
    CMatrix	result(*this) ;		// copy ourselves
    // 进行减法操作
    
		arm_mat_add_f32(&MatIns_, &other.MatIns_, &result.MatIns_);

    return result ;
}

//////////////////////////////////////////////////////////////////////
// 重载运算符*，实现matrix的数乘
//
// params:
// 1. double value - 与指定matrix相乘的实数
//
// return: CMatrix型，指定matrix与value相乘之积
//////////////////////////////////////////////////////////////////////
CMatrix	CMatrix::operator*(float value) const
{
    // 构造目标matrix
    CMatrix	result(*this) ;		// copy ourselves
    // 进行数乘
		arm_mat_scale_f32(&MatIns_, value, &result.MatIns_);

    return result ;
}


//////////////////////////////////////////////////////////////////////
// 重载运算符/，实现matrix的数乘
//
// params:
// 1. double value - 与指定matrix相除的实数
//
// return: CMatrix型，指定matrix与value相除的结果
//////////////////////////////////////////////////////////////////////
CMatrix	CMatrix::operator/(float value) const
{
    // 构造目标matrix
    CMatrix	result(*this) ;		// copy ourselves
    // 进行数乘
    for (int i = 0 ; i < MatIns_.numRows ; ++i)
    {
        for (int j = 0 ; j <  MatIns_.numCols; ++j)
            result.SetElement(i, j, result.GetElement(i, j) / value) ;
    }

    return result ;
}

//////////////////////////////////////////////////////////////////////
// 重载运算符*，实现matrix的乘法
//
// params:
// 1. const CMatrix& other - 与指定matrix相乘的matrix
//
// return: CMatrix型，指定matrix与other相乘之积
//////////////////////////////////////////////////////////////////////
CMatrix	CMatrix::operator*(const CMatrix& other) const
{
    // 首先检查行列数是否符合要求
    //ASSERT (MatIns_.numCols == other.GetNumRows());

    // construct the object we are going to return
    CMatrix	result(MatIns_.numRows, other.GetNumColumns()) ;

    // matrix乘法，即
    //
    // [A][B][C]   [G][H]     [A*G + B*I + C*K][A*H + B*J + C*L]
    // [D][E][F] * [I][J] =   [D*G + E*I + F*K][D*H + E*J + F*L]
    //             [K][L]
    //
    arm_mat_mult_f32(&MatIns_, &other.MatIns_, &result.MatIns_);

    return result ;
}

float	CMatrix::operator[](int index) const
{
    return MatIns_.pData[index];
}



//////////////////////////////////////////////////////////////////////
// 复matrix的乘法
//
// params:
// 1. const CMatrix& AR - 左边复matrix的实部matrix
// 2. const CMatrix& AI - 左边复matrix的虚部matrix
// 3. const CMatrix& BR - 右边复matrix的实部matrix
// 4. const CMatrix& BI - 右边复matrix的虚部matrix
// 5. CMatrix& CR - 乘积复matrix的实部matrix
// 6. CMatrix& CI - 乘积复matrix的虚部matrix
//
// return: bool型，复matrix乘法是否成功
//////////////////////////////////////////////////////////////////////
bool CMatrix::CMul(const CMatrix& AR, const CMatrix& AI, const CMatrix& BR, const CMatrix& BI, CMatrix& CR, CMatrix& CI) const
{
    // 首先检查行列数是否符合要求
    if (AR.GetNumColumns() != AI.GetNumColumns() ||
        AR.GetNumRows() != AI.GetNumRows() ||
        BR.GetNumColumns() != BI.GetNumColumns() ||
        BR.GetNumRows() != BI.GetNumRows() ||
        AR.GetNumColumns() != BR.GetNumRows())
        return false;

    // 构造乘积matrix实部matrix和虚部matrix
    CMatrix mtxCR(AR.GetNumRows(), BR.GetNumColumns()), mtxCI(AR.GetNumRows(), BR.GetNumColumns());
    // 复matrix相乘
    for (int i=0; i<AR.GetNumRows(); ++i)
    {
        for (int j=0; j<BR.GetNumColumns(); ++j)
        {
            float vr = 0;
            float vi = 0;
            for (int k =0; k<AR.GetNumColumns(); ++k)
            {
                float p = AR.GetElement(i, k) * BR.GetElement(k, j);
                float q = AI.GetElement(i, k) * BI.GetElement(k, j);
                float s = (AR.GetElement(i, k) + AI.GetElement(i, k)) * (BR.GetElement(k, j) + BI.GetElement(k, j));
                vr += p - q;
                vi += s - p - q;
            }
            mtxCR.SetElement(i, j, vr);
            mtxCI.SetElement(i, j, vi);
        }
    }

    CR = mtxCR;
    CI = mtxCI;

    return true;
}

//////////////////////////////////////////////////////////////////////
// matrix的转置
//
// params:无
//
// return: CMatrix型，指定matrix转置matrix
//////////////////////////////////////////////////////////////////////
CMatrix CMatrix::Transpose() const
{
    // 构造目标matrix
    CMatrix	Trans(MatIns_.numCols, MatIns_.numRows);
    // 转置各元素
	
		arm_mat_trans_f32(&MatIns_, &Trans.MatIns_);
    return Trans;
}

//////////////////////////////////////////////////////////////////////
// 实matrix求逆的全选主元高斯－约当法
//
// params:无
//
// return: boolean. Wether invert is success
//////////////////////////////////////////////////////////////////////
bool CMatrix::InvertGaussJordan()
{
	CMatrix	Inv(MatIns_.numCols, MatIns_.numRows);
	arm_status status = arm_mat_inverse_f32(&MatIns_, &Inv.MatIns_);
	*this = Inv;
 
	return (ARM_MATH_SUCCESS == status);
}


bool CMatrix::print(void)
{
    int i,j;
    for(i=0; i< MatIns_.numRows; i++){
        for(j = 0; j < MatIns_.numCols; j++){
					Console::Instance()->printf("%f ", GetElement(i, j));
        }
        Console::Instance()->printf("\r\n");
    }
    Console::Instance()->printf("\r\n");

    return false;
}

// Get the sub matrix from the parents' matrix
CMatrix CMatrix::GetSubMatrix(int row1, int row2, int col1, int col2)
{
    int row, col;
    if(row2 < row1 || col2 < col1){
			Console::Instance()->printf("Error sub matrix\r\n");
        return CMatrix();
    }
    row = row2 - row1 + 1;
    col = col2 - col1 + 1;
    CMatrix result(row, col);

    for(int i = 0; i < row; i++){
        for(int j  = 0; j < col; j++){
            result.SetElement(i,j, GetElement(row1+i, col1+j));
        }
    }
    return result;
}

float CMatrix::norm(void) const
{
    float result = 0;
    for(int i = 0; i < MatIns_.numCols * MatIns_.numRows; i++)
    {
        result += MatIns_.pData[i] * MatIns_.pData[i];
    }
    return sqrt(result);
}


CMatrix CMatrix::GetColVectorMat(int nCol)
{
    CMatrix result(MatIns_.numRows, 1);

    for (int i=0; i<MatIns_.numRows; ++i)
        result.SetElement(i, 0,  GetElement(i, nCol));

    return result;
}
