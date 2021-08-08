// Inventory.cpp: 实现文件
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "Inventory.h"
#include "afxdialogex.h"
#include <stdio.h>  
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */
#include "resource.h"

int amount, //单次订货量
bigs, initial_inv_level, inv_level, inv_next_event_type, inv_num_events,
num_months, num_values_demand,//需求规模数量 
smalls, num_policies;
float area_holding, area_shortage, holding_cost, incremental_cost, maxlag,
mean_interdemand, minlag, prob_distrib_demand[26], setup_cost,
shortage_cost, inv_sim_time, inv_time_last_event, inv_time_next_event[5],
total_ordering_cost;
int replication;
//第三题变量
int check_express;
float setup_cost_express, incremental_cost_express, minlag_express, maxlag_express;
int num_express = 0;//快速订单的数量
float backlog_time = 0.0;//缺货时间

//第四题变量
int check_perishable;
float minlag_perishable, maxlag_perishable;
int perishable_goods_inv;//易逝品库存数量
int discarded_num = 0;//被丢弃的数量
float Total_shelf_life[100];//存储生鲜时间
int Total_Arrival;//总到货量


int inv_numSeed = 0;
int inv_seed[100] = { 0 };

// Inventory 对话框

void  inv_initialize(int);
void  inv_timing(void);
void  order_arrival(void);
void  demand(int);
void  evaluate(void);
void  inv_update_time_avg_stats(void);
float inv_expon(float mean, int seed);
int   random_integer(float prob_distrib[]);
float uniform(float a, float b); //(a，b)均匀分布

IMPLEMENT_DYNAMIC(Inventory, CDialogEx)

Inventory::Inventory(CWnd* pParent /*=nullptr*/)//初始化数据
	: CDialogEx(IDD_DIALOG4, pParent)
	, m_num_values_demand(4)//number of demand size
	, m_mean_interdemand_time(0.1)//times between demand,IID exponential random
	, m_holding_cost(1)
	, m_shortage_cost(5)
	, m_setup_cost(32)
	, m_incremental_cost(3)
	, m_maxlag(1)
	, m_minlag(0.5)
	, m_initial_inv_level(60)
	, m_simulation_length(120)
	, m_policy_number(9)
	, m_min_inventory(20)
	, m_max_inventory(100)
	, m_interval(20)
	, m_replication(1)
	, m_check_express(FALSE)
	, m_check_perishable(FALSE)
{

}

Inventory::~Inventory()
{
}

void Inventory::DoDataExchange(CDataExchange* pDX)//添加变量
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT11, m_num_values_demand);
	DDX_Text(pDX, IDC_EDIT12, m_mean_interdemand_time);
	DDX_Text(pDX, IDC_EDIT13, m_holding_cost);
	DDX_Text(pDX, IDC_EDIT14, m_shortage_cost);
	DDX_Text(pDX, IDC_EDIT15, m_setup_cost);
	DDX_Text(pDX, IDC_EDIT16, m_incremental_cost);
	DDX_Text(pDX, IDC_EDIT17, m_maxlag);
	DDX_Text(pDX, IDC_EDIT18, m_minlag);
	DDX_Text(pDX, IDC_EDIT2, m_seed);
	DDX_Text(pDX, IDC_EDIT3, m_initial_inv_level);
	DDX_Text(pDX, IDC_EDIT4, m_simulation_length);
	DDX_Text(pDX, IDC_EDIT5, m_policy_number);
	DDX_Text(pDX, IDC_EDIT6, m_min_inventory);
	DDX_Text(pDX, IDC_EDIT9, m_max_inventory);
	DDX_Control(pDX, IDC_LIST2, m_inventory);
	DDX_Control(pDX, IDC_LIST5, m_demend_distribution);
	DDX_Text(pDX, IDC_EDIT10, m_interval);
	DDX_Text(pDX, IDC_EDIT1, m_replication);
	DDX_Control(pDX, IDC_LIST3, m_seedlist);
	DDX_Check(pDX, IDC_CHECK2, m_check_express);
	DDX_Check(pDX, IDC_CHECK1, m_check_perishable);
}


BEGIN_MESSAGE_MAP(Inventory, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &Inventory::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON7, &Inventory::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON1, &Inventory::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON6, &Inventory::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON4, &Inventory::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &Inventory::OnBnClickedButton5)

	ON_BN_CLICKED(IDC_BUTTON3, &Inventory::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON8, &Inventory::OnBnClickedButton8)
END_MESSAGE_MAP()


// Inventory 消息处理程序


BOOL Inventory::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_inventory.InsertColumn(0, _T("Sim."), LVCFMT_CENTER, 50);
	m_inventory.InsertColumn(1, _T("Seed"), LVCFMT_CENTER, 50);
	m_inventory.InsertColumn(2, _T("s"), LVCFMT_CENTER, 40);
	m_inventory.InsertColumn(3, _T("S"), LVCFMT_CENTER, 40);
	m_inventory.InsertColumn(4, _T("平均总成本"), LVCFMT_CENTER, 80);
	m_inventory.InsertColumn(5, _T("平均订货成本"), LVCFMT_CENTER, 80);
	m_inventory.InsertColumn(6, _T("平均存贮成本"), LVCFMT_CENTER, 80);
	m_inventory.InsertColumn(7, _T("平均缺货成本"), LVCFMT_CENTER, 80);

	m_demend_distribution.InsertColumn(0, _T("U"), LVCFMT_CENTER, 50);
	m_demend_distribution.InsertColumn(1, _T("Variates"), LVCFMT_CENTER, 100);

	//第三题
	m_inventory.InsertColumn(8, _T("期望缺货概率"), LVCFMT_CENTER, 80);
	m_inventory.InsertColumn(9, _T("期望加急订单数"), LVCFMT_CENTER, 80);

	//第四题
	m_inventory.InsertColumn(10, _T("丢弃率"), LVCFMT_CENTER, 60);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void Inventory::OnBnClickedButton2()//run
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(1);

	inv_num_events = 4;
	initial_inv_level = m_initial_inv_level;
	num_months = m_simulation_length;
	num_policies = m_policy_number;
	num_values_demand = m_num_values_demand;
	mean_interdemand = m_mean_interdemand_time;
	setup_cost = m_setup_cost;
	incremental_cost = m_incremental_cost;
	holding_cost = m_holding_cost;
	shortage_cost = m_shortage_cost;
	minlag = m_minlag;
	maxlag = m_maxlag;
	bigs = m_max_inventory;
	smalls = m_min_inventory;
	replication = m_replication;
	//第三题
	check_express = m_check_express;
	setup_cost_express = 48;
	incremental_cost_express = 4;
	minlag_express = 0.25;
	maxlag_express = 0.5;
	//第四题
	check_perishable = m_check_perishable;
	minlag_perishable = 1.5;
	maxlag_perishable = 2.5;


	int flag;
	int tag;
	int interval_num = (bigs - smalls) / m_interval;
	//int interval_num = (int)ceil(1.0 * (bigs - smalls) / m_interval);
	int min_smalls[100] = { 0 };
	min_smalls[0] = smalls;//定义s下限数组
	for (int i = 1; i <= interval_num - 1; ++i)
		min_smalls[i] = min_smalls[0] + m_interval * i;

	int seednum;
	for (seednum = 0; seednum < inv_numSeed; ++seednum)
	{
		flag = 0;
		for (int s = 0; s < interval_num; ++s)
		{
			smalls = min_smalls[s];
			if (s == (interval_num - 1))
			{
				interval_num = interval_num - s + 1;
			}
			else
				interval_num = interval_num - s;
			for (int j = 1; j <= interval_num; j++)
			{
				bigs = smalls + m_interval * j;

				for (int i = 1; i <= replication; ++i)
				{

					inv_initialize(inv_seed[seednum]);

					/* Run the simulation until it terminates after an end-simulation event
					   (type 3) occurs. */
					do
					{

						/* Determine the next event. */
						inv_timing();

						/* Update time-average statistical accumulators. */
						inv_update_time_avg_stats();

						/* Invoke the appropriate event function. */

						switch (inv_next_event_type)
						{
						case 1:
							order_arrival();
							break;
						case 2:
							demand(inv_seed[seednum]);
							break;
						case 4:
							evaluate();
							break;
						case 3:
							report(inv_seed[seednum]);
							break;
						}

						/* If the event just executed was not the end-simulation event (type 3),
						   continue simulating.  Otherwise, end the simulation for the current
						   (s,S) pair and go on to the next pair (if any). */

					} while (inv_next_event_type != 3);
				}
				flag++;
				if (flag == num_policies)
					goto tag;
			}
		}
	tag:;
	}



	/*   return 0;*/
}


void inv_initialize(int seed)  /* Initialization function. */
{
	/* Initialize the simulation clock. */

	inv_sim_time = 0.0;

	/* Initialize the state variables. */

	inv_level = initial_inv_level;
	inv_time_last_event = 0.0;

	/* Initialize the statistical counters. */

	total_ordering_cost = 0.0;
	area_holding = 0.0;
	area_shortage = 0.0;

	/* Initialize the event list.  Since no order is outstanding, the order-
	   arrival event is eliminated from consideration. */

	inv_time_next_event[1] = 1.0e+30;
	inv_time_next_event[2] = inv_sim_time + inv_expon(mean_interdemand, seed);
	inv_time_next_event[3] = num_months;
	inv_time_next_event[4] = 0.0;

	//第三题：
	num_express = 0;
	backlog_time = 0.0;

	//第四题
	if (check_perishable == TRUE)
	{
		discarded_num = 0;
		for (int i = 1; i <= inv_level; i++) //对初始的库存记录其生鲜时间
		{
			Total_shelf_life[i] = inv_sim_time + uniform(minlag_perishable, maxlag_perishable);
		}
	}
}


void inv_timing(void)  /* Timing function. */
{
	int   i;
	float inv_min_time_next_event = 1.0e+29;

	inv_next_event_type = 0;

	/* Determine the event type of the next event to occur. */

	for (i = 1; i <= inv_num_events; ++i)
		if (inv_time_next_event[i] < inv_min_time_next_event)
		{
			inv_min_time_next_event = inv_time_next_event[i];
			inv_next_event_type = i;
		}

	/* Check to see whether the event list is empty. */

	if (inv_next_event_type == 0) {

		/* The event list is empty, so stop the simulation */
		exit(1);
	}

	/* The event list is not empty, so advance the simulation clock. */

	inv_sim_time = inv_min_time_next_event;
}


void order_arrival(void)  /* Order arrival event function. */
{
	int a = inv_level;
	/* Increment the inventory level by the amount ordered. */
	if (check_perishable == TRUE)
	{
		/*if (inv_level >= 0)*/
		//{
		inv_level += amount;
		for (int i = a + 1; i <= inv_level; i++) //对到达的生鲜产品，记录其过期时间
		{
			Total_shelf_life[i] = inv_sim_time + uniform(minlag_perishable, maxlag_perishable);
		}
		//}
		//else//库存不足，先补齐需求，再记录剩下的生鲜产品时间
		//{
		//	for (int i = 1; i <= amount + inv_level; i++)//
		//	{
		//		Total_shelf_life[i] = inv_sim_time + uniform(minlag_perishable, maxlag_perishable);
		//	}
		//}
	}
	else {
		inv_level += amount;
	}
	Total_Arrival += amount;//记录总订货量
	/* 由于现在没有未完成的订单，将订单到达事件从考虑中删除。 */

	inv_time_next_event[1] = 1.0e+30;
}


void demand(int seed)  /* Demand event function. */
{
	int demand_num = random_integer(prob_distrib_demand);
	int i = 1;
	/* Decrement the inventory level by a generated demand size. */
	if (check_perishable == TRUE)
	{

		while (i <= inv_level && i <= demand_num)
		{
			if (Total_shelf_life[i] > inv_sim_time)
				i++;
			else
			{
				int flag = i;
				discarded_num++;
				for (int j = flag; j <= inv_level; j++)//数组移位
				{
					Total_shelf_life[j] = Total_shelf_life[j + 1];
				}
				inv_level--;
			}
		}

	}
	//if (inv_level >= demand_num)//可以满足demand_num
 //   {
	//         for (int i = 1; i <= inv_level-demand_num; i++)//更新库存
	//	     Total_shelf_life[i] = Total_shelf_life[i+demand_num];
	//         inv_level = inv_level - demand_num;
 //   }
 //   else//库存不能满足demand_num
 //   {
	//         for (int i = 1; i <= inv_level; i++)
	//	     Total_shelf_life[i] = 0.0;
	//         inv_level = inv_level - demand_num;
 //   }
	//

	else {
		inv_level -= demand_num;
	}

	/* Schedule the time of the next demand. */

	inv_time_next_event[2] = inv_sim_time + inv_expon(mean_interdemand, seed);
}


void evaluate(void)  /* Inventory-evaluation event function. */
{
	/* Check whether the inventory level is less than smalls. */
	if (check_express == TRUE)
	{
		if (inv_level < 0)
		{
			amount = bigs - inv_level;
			total_ordering_cost += setup_cost_express + incremental_cost_express * amount;
			num_express++;
			inv_time_next_event[1] = inv_sim_time + uniform(minlag_express, maxlag_express);
		}
		else if ((inv_level >= 0) && (inv_level <= smalls))
		{
			/* The inventory level is less than smalls, so place an order for the
			   appropriate amount. */

			amount = bigs - inv_level;
			total_ordering_cost += setup_cost + incremental_cost * amount;

			/* Schedule the arrival of the order. */

			inv_time_next_event[1] = inv_sim_time + uniform(minlag, maxlag);
		}
	}
	else
	{
		if (inv_level < smalls)
		{

			/* The inventory level is less than smalls, so place an order for the
			   appropriate amount. */

			amount = bigs - inv_level;
			total_ordering_cost += setup_cost + incremental_cost * amount;

			/* Schedule the arrival of the order. */

			inv_time_next_event[1] = inv_sim_time + uniform(minlag, maxlag);
		}
	}


	/* Regardless of the place-order decision, schedule the next inventory
	   evaluation. */

	inv_time_next_event[4] = inv_sim_time + 1.0;
}


void inv_update_time_avg_stats(void)  /* Update area accumulators for time-average
									 statistics. */
{
	float time_since_last_event;//自上次事件以来所经过的时间

	/* Compute time since last event, and update last-event-time marker. */

	time_since_last_event = inv_sim_time - inv_time_last_event;//自上次事件以来所经过的时间=当前仿真时间-上次事件的事件
	inv_time_last_event = inv_sim_time;

	/* Determine the status of the inventory level during the previous interval.
	   If the inventory level during the previous interval was negative, update
	   area_shortage.  If it was positive, update area_holding.  If it was zero,
	   no update is needed. */

	if (inv_level < 0)
	{
		area_shortage -= inv_level * time_since_last_event;
		backlog_time = backlog_time + time_since_last_event;
	}

	else if (inv_level > 0)
		area_holding += inv_level * time_since_last_event;
}


float inv_expon(float mean, int seed)  /* Exponential variate generation function. */
{
	/* Return an exponential random variate with mean "mean". */

	return -mean * log(lcgrand(seed));//您需要查找 disp_pentium4.inc 以通过查看源来确定当前调用堆栈帧。
}


int random_integer(float prob_distrib[])  /* Random integer generation
											 function. */
{
	int   i;
	float u;

	/* Generate a U(0,1) random variate. */

	u = lcgrand(1);

	/* Return a random integer in accordance with the (cumulative) distribution
	   function prob_distrib. */

	for (i = 1; u >= prob_distrib[i]; ++i)
		;
	return i;
}


float uniform(float a, float b)  /* Uniform variate generation function. */
{
	/* Return a U(a,b) random variate. */

	return a + lcgrand(1) * (b - a);
}

int p = 0;
int Inventory::report(int seed)
{
	// TODO: 在此处添加实现代码.
	CString str;
	float avg_holding_cost, avg_ordering_cost, avg_shortage_cost;
	avg_ordering_cost = total_ordering_cost / num_months;
	avg_holding_cost = holding_cost * area_holding / num_months;
	avg_shortage_cost = shortage_cost * area_shortage / num_months;

	str.Format(TEXT("%d"), p + 1);
	m_inventory.InsertItem(0, str);
	str.Format(TEXT("%d"), seed);
	m_inventory.SetItemText(0, 1, str);
	str.Format(TEXT("%d"), smalls);
	m_inventory.SetItemText(0, 2, str);
	str.Format(TEXT("%d"), bigs);
	m_inventory.SetItemText(0, 3, str);
	str.Format(TEXT("%.4f"), avg_ordering_cost + avg_holding_cost + avg_shortage_cost);//average total cost
	m_inventory.SetItemText(0, 4, str);
	str.Format(TEXT("%.4f"), avg_ordering_cost);//average order cost
	m_inventory.SetItemText(0, 5, str);
	str.Format(TEXT("%.4f"), avg_holding_cost);//average holding cost
	m_inventory.SetItemText(0, 6, str);
	str.Format(TEXT("%.4f"), avg_shortage_cost);//average shortage cost
	m_inventory.SetItemText(0, 7, str);

	//第四题
	str.Format(TEXT("%.4f"), 1.0 * discarded_num / Total_Arrival);//Expected number of express
	m_inventory.SetItemText(0, 10, str);

	//第三题
	str.Format(TEXT("%.4f"), backlog_time / inv_sim_time);//Expected proportion of backlog
	m_inventory.SetItemText(0, 8, str);
	str.Format(TEXT("%d"), num_express);//Expected number of express
	m_inventory.SetItemText(0, 9, str);

	++p;

	return 0;
}


void Inventory::OnBnClickedButton7()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(1);
	num_values_demand = m_num_values_demand;//表示需求数量的类型值

	prob_distrib_demand[1] = 0.167;
	prob_distrib_demand[2] = 0.500;
	prob_distrib_demand[3] = 0.833;
	prob_distrib_demand[4] = 1.000;
	int a[100] = { 0,1,2,3,4,5 };

	for (int i = 0; i < num_values_demand; ++i) {

		CString str;
		str.Format(TEXT("%3d"), a[i + 1]);
		m_demend_distribution.InsertItem(i, str);

		str.Format(TEXT("%.3f"), prob_distrib_demand[i + 1]);
		m_demend_distribution.SetItemText(i, 1, str);
	}
}


void Inventory::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_inventory.DeleteAllItems();
}


void Inventory::OnBnClickedButton6()//添加种子
{
	// TODO: 在此添加控件通知处理程序代码

	UpdateData(1);

	CString str;
	str.Format(TEXT("%d"), m_seed);     //int 型转变为字符型

	m_seedlist.InsertItem(inv_numSeed, str);
	inv_seed[inv_numSeed] = m_seed;
	inv_numSeed++;
}


void Inventory::OnBnClickedButton4()//清空所有的种子数
{
	// TODO: 在此添加控件通知处理程序代码
	m_seedlist.DeleteAllItems();
	inv_numSeed = 0;
}

static long zrng[] =
{ 1,
 1973272912, 281629770, 20006270,1280689831,2096730329,1933576050,
 913566091, 246780520,1363774876, 604901985,1511192140,1259851944,
 824064364, 150493284, 242708531, 75253171,1964472944,1202299975,
 233217322,1911216000, 726370533, 403498145, 993232223,1103205531,
 762430696,1922803170,1385516923, 76271663, 413682397, 726466604,
 336157058,1432650381,1120463904, 595778810, 877722890,1046574445,
 68911991,2088367019, 748545416, 622401386,2122378830, 640690903,
 1774806513,2132545692,2079249579, 78130110, 852776735,1187867272,
 1351423507,1645973084,1997049139, 922510944,2045512870, 898585771,
 243649545,1004818771, 773686062, 403188473, 372279877,1901633463,
 498067494,2087759558, 493157915, 597104727,1530940798,1814496276,
 536444882,1663153658, 855503735, 67784357,1432404475, 619691088,
 119025595, 880802310, 176192644,1116780070, 277854671,1366580350,
 1142483975,2026948561,1053920743, 786262391,1792203830,1494667770,
  1923011392,1433700034,1244184613,1147297105, 539712780,1545929719,
 190641742,1645390429, 264907697, 620389253,1502074852, 927711160,
 364849192,2049576050, 638580085, 547070247 };



void Inventory::OnBnClickedButton5()//重置随机数流
{
	// TODO: 在此添加控件通知处理程序代码
	for (int i = 0; i < 101; i++)
	{
		lcgrandst(zrng[i], i);
	}
}




void Inventory::OnBnClickedCheck2()
{
	// TODO: 在此添加控件通知处理程序代码
}


void Inventory::OnBnClickedButton3()//删除选中的种子数
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_seedlist.GetSelectedCount() > 0)
	{
		POSITION pos = m_seedlist.GetFirstSelectedItemPosition();
		while (pos)
		{
			int nSelected = m_seedlist.GetNextSelectedItem(pos);//获取选中行的索引
			m_seedlist.DeleteItem(nSelected);//根据索引删除
			pos = m_seedlist.GetFirstSelectedItemPosition();
			inv_numSeed--;
		}
	}
}


void Inventory::OnBnClickedButton8()
{
	// TODO: 在此添加控件通知处理程序代码
	ShellExecute(NULL, _T("open"), _T("Report3.pdf"), NULL, NULL, SW_SHOWNORMAL);
}
