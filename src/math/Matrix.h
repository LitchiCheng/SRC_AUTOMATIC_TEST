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

    CMatrix();													// �������캯��
    CMatrix(int nRows, int nCols);					// ָ�����й��캯��
    CMatrix(int nRows, int nCols,const float value[]);	// ָ�����ݹ��캯��
		CMatrix(int nRows, int nCols,float* pData);	//Constructor for CMSIS
    CMatrix(int nSize);								// �����캯��
    CMatrix(int nSize, const float value[]);				// ָ�����ݷ����캯��
    CMatrix(const CMatrix& other);					// �������캯��
    bool	Init(int nRows, int nCols);				// ��ʼ������
    bool	MakeUnitMatrix(int nSize);				// �������ʼ��Ϊ��λ����
		bool	MakeUnitMatrix();				// �������ʼ��Ϊ��λ����
    virtual ~CMatrix();								// ��������

    // Output and display
    bool print(void);

    //
    // operation of elements and value
    //

    bool	SetElement(int nRow, int nCol,float value);	// ����ָ��Ԫ�ص�ֵ
    float	GetElement(int nRow, int nCol) const;			// ��ȡָ��Ԫ�ص�ֵ
    void    SetData(const float value[]);						// ���þ����ֵ
    void	SetDataF(float value[], int nRows, int nCols);	// ���þ����ֵ������Fortranר�á����ã���
    void    SetDataVertical(float value[]);						// ���þ����ֵ��ֱ����
    int		GetNumColumns() const;							// ��ȡ���������
    int		GetNumRows() const;								// ��ȡ���������
//    int     GetRowVector(int nRow, float* pVector) const;	// ��ȡ�����ָ���о���
//    int     GetColVector(int nCol, float* pVector) const;	// ��ȡ�����ָ���о���
    float* GetData() const;								// ��ȡ�����ֵ

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
    // ������˷�
    bool CMul(const CMatrix& AR, const CMatrix& AI, const CMatrix& BR, const CMatrix& BI, CMatrix& CR, CMatrix& CI) const;
    // �����ת��
    CMatrix Transpose() const;

    float operator[](int index) const;

    //
    // �㷨
    //

    // ʵ���������ȫѡ��Ԫ��˹��Լ����
    bool InvertGaussJordan();


    // Get the sub matrix from the parents' matrix
    CMatrix GetSubMatrix(int row1, int row2, int col1, int col2);

    float norm(void) const;


    CMatrix GetColVectorMat(int nCol);


    //
    // ���������ݳ�Ա
    //
protected:
		arm_matrix_instance_f32 MatIns_;

    //
    // �ڲ�����
    //
private:
    void ppp(float a[], float e[], float s[], float v[], int m, int n);
    void sss(float fg[2], float cs[2]);


};


#endif
