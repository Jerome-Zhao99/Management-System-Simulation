// Coin.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "Coin.h"
#include "afxdialogex.h"
#include "lcgrand.h"


// Coin 对话框

IMPLEMENT_DYNAMIC(Coin, CDialogEx)

Coin::Coin(CWnd* pParent /*=NULL*/)
	: CDialogEx(Coin::IDD, pParent)
	, m_cost(1)
	, m_condition(3)
	, m_revenue(8)
	, m_flip(0)
	, m_profit(0)
	, m_replication(0)
{

}

Coin::~Coin()
{
}

void Coin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_cost);
	DDX_Text(pDX, IDC_EDIT2, m_condition);
	DDX_Text(pDX, IDC_EDIT3, m_revenue);
	DDX_Text(pDX, IDC_EDIT4, m_flip);
	DDX_Text(pDX, IDC_EDIT5, m_profit);
	DDX_Text(pDX, IDC_EDIT6, m_replication);
}


BEGIN_MESSAGE_MAP(Coin, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &Coin::OnBnClickedButton1)

END_MESSAGE_MAP()


// Coin 消息处理程序


void Coin::OnBnClickedButton1()
{
	UpdateData(1);
	int condition=0;
	int flip=0;
	int replication=0;
	float sum_flips =0.0;

	

	for(replication =0; replication < m_replication; replication++)
	{
		flip=0;
		condition=0;
		
		do
		{
		//产生随机数
			
			float random_number=lcgrand(1);
			flip += 1;
			if(random_number<0.5)
				condition+=1;
			else
				condition-=1;
		}
		while(abs(condition)!=m_condition);

		sum_flips = sum_flips +flip;
	};

	m_flip= sum_flips / m_replication;
	m_profit=m_revenue-m_flip*m_cost;

	UpdateData(0);
}




void Coin::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	ShellExecute(NULL,_T("open"),_T("Report1.pdf"),NULL,NULL,SW_SHOWNORMAL);
}

