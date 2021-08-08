// Queue.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "Queue.h"
#include "afxdialogex.h"
#include <stdio.h>  
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */
#include "stdafx.h"
#include "resource.h"

#define Q_LIMIT 100   /* Limit on queue length. */
#define BUSY      1  /* Mnemonics for server's being busy */
#define IDLE      0   /* and idle. */

int   next_event_type, num_custs_delayed, num_delays_required, num_events,
num_in_q, server_status;//������һ�¼����ͣ��ͻ��ȴ����������¼��������ӳ�������״̬
float area_num_in_q, area_server_status, mean_interarrival, mean_service,
sim_time, time_arrival[Q_LIMIT + 1], time_last_event, time_next_event[4],//��һ�����¼��ܳ���Ϊ3
total_of_delays;

int   max_queue_length,  //���ӳ�
num_over_delay; //�����ȴ�ʱ��Ĺ˿�����
float max_delay_time, //����Ŷ�ʱ��
max_sys_delay_time,  //���ϵͳ�ȴ�ʱ��
delay_threshold;//�Ŷ�ʱ�䳬����ʱ��(��λ������)
float openning_time_limit,  //ָ������ʱ��
operation_time_limit; //ָ��Ӫҵʱ��
int queue_length_limit, //�ӳ�����
radio1,
radio2;
int no = 0;
int numSeed = 0;
int seed[100];
int balk_num=0;

//��������
void  initialize(int n_seed);//��ʼ��
void  timing(void);//������
void  arrive(int n_seed);//�˿͵����¼�
void  depart(int n_seed);//�˿��뿪�¼�
void  main(int radio, int n_seed);//������
void  update_time_avg_stats(void);//����ʱ���ڵ�ƽ�����ݣ����֣�
float expon(float mean, int seed);//�����


// Queue �Ի���

IMPLEMENT_DYNAMIC(Queue, CDialogEx)

Queue::Queue(CWnd* pParent /*=NULL*/)
	: CDialogEx(Queue::IDD, pParent)
	, m_mean_interarrival(1)
	, m_mean_service(0.5)
	, m_num_delays_required(10000)//��Ҫ�ŶӵĹ˿��� Number of customer
	, m_delay_threshold(1)//�Ŷ�ʱ�䳬��1���ӵ�ʱ��(��λ������) Excessing delay time
	, m_limit_num_in_q(2)//�ӳ����ƣ���ʼֵΪ2 Queue length
	, m_time_limit(480)//ָ�����Ž��ܹ˿ͽ���ʱ��
	, m_time_operation(480)//ָ��Ӫҵʱ��
	, m_radio1(0)//ǰ����radio button����ʼ��Ϊѡ���һ��
	, m_radio2(0)//������radio button����ʼ��Ϊѡ��ڶ���
	, m_seed(0)

{

}

Queue::~Queue()
{
}

void Queue::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_mean_interarrival);
	DDX_Text(pDX, IDC_EDIT2, m_mean_service);
	DDX_Text(pDX, IDC_EDIT3, m_num_delays_required);
	DDX_Control(pDX, IDC_LIST1, m_queue);
	DDX_Text(pDX, IDC_EDIT4, m_delay_threshold);
	DDX_Text(pDX, IDC_EDIT5, m_limit_num_in_q);
	DDX_Text(pDX, IDC_EDIT7, m_time_limit);
	DDX_Text(pDX, IDC_EDIT8, m_time_operation);
	DDX_Control(pDX, IDC_LIST2, m_SeedList);
	DDX_Text(pDX, IDC_EDIT6, m_seed);
	DDX_Radio(pDX, IDC_RADIO13, m_radio1);
	DDX_Radio(pDX, IDC_RADIO15, m_radio2);
}


BEGIN_MESSAGE_MAP(Queue, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &Queue::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_RADIO17, &Queue::OnBnClickedRadio17)
	ON_BN_CLICKED(IDC_RADIO13, &Queue::OnBnClickedRadio13)
	ON_BN_CLICKED(IDC_RADIO14, &Queue::OnBnClickedRadio14)
	ON_BN_CLICKED(IDC_RADIO15, &Queue::OnBnClickedRadio15)
	ON_BN_CLICKED(IDC_RADIO16, &Queue::OnBnClickedRadio16)
	ON_BN_CLICKED(IDC_BUTTON4, &Queue::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON7, &Queue::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &Queue::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &Queue::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON6, &Queue::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON5, &Queue::OnBnClickedButton5)
END_MESSAGE_MAP()




BOOL Queue::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_queue.InsertColumn(0, TEXT("Simulation"), LVCFMT_CENTER, 90);
	m_queue.InsertColumn(1, TEXT("Seed"), LVCFMT_CENTER, 50);

	m_queue.InsertColumn(2, TEXT("Average delay in queu"), LVCFMT_CENTER, 130);
	m_queue.InsertColumn(3, TEXT("Average number in queue"), LVCFMT_CENTER, 90);
	m_queue.InsertColumn(4, TEXT("Server utilization"), LVCFMT_CENTER, 130);

	m_queue.InsertColumn(5, TEXT("Average number in queue system"), LVCFMT_CENTER, 150);
	m_queue.InsertColumn(6, TEXT("Average delay in system"), LVCFMT_CENTER, 150);
	m_queue.InsertColumn(7, TEXT("Maximum queue length"), LVCFMT_CENTER, 90);
	m_queue.InsertColumn(8, TEXT("Maximum delay time in queue"), LVCFMT_CENTER, 150);
	m_queue.InsertColumn(9, TEXT("Maximum delay time in system"), LVCFMT_CENTER, 150);
	m_queue.InsertColumn(10, TEXT("Proportion of customers delay in queue over 1 minute"), LVCFMT_CENTER, 130);
	m_queue.InsertColumn(11, TEXT("Balk number"), LVCFMT_CENTER, 130);

	m_queue.InsertColumn(12, TEXT("Time of simulation"), LVCFMT_CENTER, 110);

	GetDlgItem(IDC_EDIT3)->EnableWindow(1);
	GetDlgItem(IDC_EDIT7)->EnableWindow(0);
	GetDlgItem(IDC_EDIT8)->EnableWindow(0);

	return TRUE;
}


void Queue::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	mean_interarrival = m_mean_interarrival;
	mean_service = m_mean_service;
	delay_threshold = m_delay_threshold;
	num_delays_required = m_num_delays_required;
	operation_time_limit = m_time_operation;
	queue_length_limit = m_limit_num_in_q;
	openning_time_limit = m_time_limit;
	radio1 = m_radio1;
	radio2 = m_radio2;
	int now_seed;
	for (int i = 0; i < numSeed; i++)
	{
		now_seed = seed[i];
		initialize(now_seed);
		//ִ�з���
		main(radio2, now_seed);
		report(i);
	}


}
bool flag;
void main(int radio, int n_seed) {
	if (radio == 0) {
		while (num_custs_delayed < num_delays_required)
		{
			/* Determine the next event. */

			timing();

			/* Update time-average statistical accumulators. */

			update_time_avg_stats();

			/* Invoke the appropriate event function. */

			switch (next_event_type)
			{
			case 1:
				arrive(n_seed);
				break;
			case 2:
				depart(n_seed);
				break;
			}
		}
	}
	else if (radio == 1)
	{
		while (flag)
		{
			timing();
			update_time_avg_stats();
			switch (next_event_type)
			{
			case 0://��һ�¼�Ϊ��
				break;
			case 1:	//�����¼�
				arrive(n_seed);
				break;
			case 2://�뿪�¼�
				depart(n_seed);
				break;
			case 3:	//�����������ʱ��
					//��֤�Ѿ��ڷ��������еĹ˿ͽ��ܷ���
				time_next_event[3] = 1.0e+30;
				//ֹͣ�˿ͽ��룬û�е����¼�
				time_next_event[1] = 1.0e+30;
				break;
			}
		};
	}
	else if (radio == 2)
	{   //ָ��Ӫҵʱ��
		while (flag)
		{
			timing();
			update_time_avg_stats();
			switch (next_event_type)
			{
			case 1:
				//���̵�δ���ţ�ֹͣ�˿ͽ���		
				arrive(n_seed);
				break;
			case 2:
				depart(n_seed);
				break;
			case 3://������Ӫʱ�䣬������Ӫ
				flag = false;
				break;

			}
		};
	}
}

void initialize(int n_seed)  /* Initialization function. */
{
	/* Initialize the simulation clock. */

	sim_time = 0.0;

	/* Initialize the state variables. */

	server_status = IDLE;//����̨״̬
	num_in_q = 0;//����������
	time_last_event = 0.0;//��һ�¼�����ʱ��

	/* Initialize the statistical counters. */

	num_custs_delayed = 0;
	total_of_delays = 0.0;
	area_num_in_q = 0.0;
	area_server_status = 0.0;
	max_queue_length = 0;
	max_delay_time = 0;
	max_sys_delay_time = 0;
	num_over_delay = 0;
	balk_num = 0;


	/* Initialize event list.  Since no customers are present, the departure
	   (service completion) event is eliminated from consideration. */

	time_next_event[1] = sim_time + expon(mean_interarrival, n_seed);
	time_next_event[2] = 1.0e+30;

	//ѡ���¼�����
	if (radio2 == 0)
		num_events = 2;
	if (radio2 == 1)
	{
		num_events = 3;
		time_next_event[3] = openning_time_limit;
	}
	if (radio2 == 2)
	{
		num_events = 3;
		time_next_event[3] = operation_time_limit;
	}

	flag = true;//ѭ��ִ��
}


void timing(void)  /* Timing function. */
{
	int   i;
	float min_time_next_event = 1.0e+29;

	next_event_type = 0;

	/* Determine the event type of the next event to occur. */

	for (i = 1; i <= num_events; ++i)
		if (time_next_event[i] < min_time_next_event)
		{
			min_time_next_event = time_next_event[i];
			next_event_type = i;
		}

	/* Check to see whether the event list is empty. */
	if (next_event_type == 0)
	{

		flag = false;
	}
	else
		sim_time = min_time_next_event;//�����ӵ�������һ�¼��ķ���ʱ��
}


void arrive(int n_seed)  /* Arrival event function. */
{

	float delay;

	/* Schedule next arrival. */

	time_next_event[1] = sim_time + expon(mean_interarrival, n_seed);

	/* Check to see whether server is busy. */

	if (server_status == BUSY)
	{

		if (radio1 == 0 && num_in_q >= queue_length_limit)
		{
			balk_num++;
			return;
		};

		/* Server is busy, so increment number of customers in queue. */
		++num_in_q;

		/* Check to see whether an overflow condition exists. */

		if (num_in_q > Q_LIMIT)
		{
			/* The queue has overflowed, so stop the simulation. */
			exit(2);
		}

		if (num_in_q > max_queue_length)
			max_queue_length = num_in_q;


		/* There is still room in the queue, so store the time of arrival of the
		   arriving customer at the (new) end of time_arrival. */

		time_arrival[num_in_q] = sim_time;
	}

	else
	{
		/* Server is idle, so arriving customer has a delay of zero.  (The
			   following two statements are for program clarity and do not affect
			   the results of the simulation.) */
		delay = 0.0;
		total_of_delays += delay;

		/* Increment the number of customers delayed, and make server busy. */

		++num_custs_delayed;
		server_status = BUSY;

		/* Schedule a departure (service completion). */

		time_next_event[2] = sim_time + expon(mean_service, n_seed);

		if (time_next_event[2] - sim_time >= max_sys_delay_time)
			max_sys_delay_time = time_next_event[2] - sim_time;

	}
}


void depart(int n_seed)  /* Departure event function. */
{
	int   i;
	float delay;//�Ŷ�ʱ��

	/* Check to see whether the queue is empty. */

	if (num_in_q == 0)
	{
		/* The queue is empty so make the server idle and eliminate the
		   departure (service completion) event from consideration. */

		server_status = IDLE;
		time_next_event[2] = 1.0e+30;
	}

	else
	{
		/* The queue is nonempty, so decrement the number of customers in
		   queue. */

		--num_in_q;

		/* Compute the delay of the customer who is beginning service and update
		   the total delay accumulator. */

		delay = sim_time - time_arrival[1];//0��Ϊ����ʹ��
		total_of_delays += delay;

		if (delay >= max_delay_time)
			max_delay_time = delay;
		if (delay >= delay_threshold)
			++num_over_delay;

		/* Increment the number of customers delayed, and schedule departure. */

		++num_custs_delayed;
		time_next_event[2] = sim_time + expon(mean_service, n_seed);

		if (time_next_event[2] - sim_time + delay >= max_sys_delay_time)
			max_sys_delay_time = time_next_event[2] - sim_time + delay;

		/* Move each customer in queue (if any) up one place. */

		for (i = 1; i <= num_in_q; ++i)
			time_arrival[i] = time_arrival[i + 1];
	}
}

void update_time_avg_stats(void) /* Update area accumulators for time-average
									 statistics. */
{
	float time_since_last_event;

	/* Compute time since last event, and update last-event-time marker. */

	time_since_last_event = sim_time - time_last_event;
	time_last_event = sim_time;

	/* Update area under number-in-queue function. */

	area_num_in_q += num_in_q * time_since_last_event;

	/* Update area under server-busy indicator function. */

	area_server_status += server_status * time_since_last_event;
}


float expon(float mean, int seed)  /* Exponential variate generation function. */
{
	/* Return an exponential random variate with mean "mean". */

	return -mean * log(lcgrand(seed));
}

void Queue::report(int i)
{

	CString str;
	str.Format(TEXT("%d"), no);
	m_queue.InsertItem(i, str);
	//��1�⡢��2��
	str.Format(TEXT("%d"), seed[i]);
	m_queue.SetItemText(i, 1, str);
	str.Format(TEXT("%.4f"), total_of_delays / num_custs_delayed);
	m_queue.SetItemText(i, 2, str);
	str.Format(TEXT("%.4f"), area_num_in_q / sim_time);
	m_queue.SetItemText(i, 3, str);
	str.Format(TEXT("%.4f"), area_server_status / sim_time);
	m_queue.SetItemText(i, 4, str);

	//��3�����
	str.Format(TEXT("%.4f"), area_num_in_q / sim_time + area_server_status / sim_time);
	m_queue.SetItemText(i, 5, str);
	str.Format(TEXT("%.4f"), total_of_delays / num_custs_delayed + area_server_status / sim_time);
	m_queue.SetItemText(i, 6, str);
	str.Format(TEXT("%d"), max_queue_length);
	m_queue.SetItemText(i, 7, str);
	str.Format(TEXT("%.4f"), max_delay_time);//Maximum time in queue
	m_queue.SetItemText(i, 8, str);
	str.Format(TEXT("%.4f"), max_sys_delay_time);//Maximum delay time in system
	m_queue.SetItemText(i, 9, str);
	str.Format(TEXT("%.4f"), 1.0 * num_over_delay / num_custs_delayed);//Proportion of customers delay in queue over 1 minute
	m_queue.SetItemText(i, 10, str);
	str.Format(TEXT("%d"), balk_num);//Proportion of customers delay in queue over 1 minute
	m_queue.SetItemText(i, 11, str);

	//�����¼����
	str.Format(TEXT("%.4f"), sim_time);
	m_queue.SetItemText(i, 12, str);

	++no;

}


void Queue::OnBnClickedRadio13()
{
	GetDlgItem(IDC_EDIT5)->EnableWindow(1);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void Queue::OnBnClickedRadio14()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_EDIT5)->EnableWindow(0);
}


void Queue::OnBnClickedRadio15()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_EDIT3)->EnableWindow(1);
	GetDlgItem(IDC_EDIT7)->EnableWindow(0);
	GetDlgItem(IDC_EDIT8)->EnableWindow(0);
}


void Queue::OnBnClickedRadio16()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_EDIT3)->EnableWindow(0);
	GetDlgItem(IDC_EDIT7)->EnableWindow(1);
	GetDlgItem(IDC_EDIT8)->EnableWindow(0);
}

void Queue::OnBnClickedRadio17()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_EDIT3)->EnableWindow(0);
	GetDlgItem(IDC_EDIT7)->EnableWindow(0);
	GetDlgItem(IDC_EDIT8)->EnableWindow(1);
}



void Queue::OnBnClickedButton4()//�������
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(1);
	CString str;

	str.Format(TEXT("%d"), m_seed);
	m_SeedList.AddString(str);
	seed[numSeed] = m_seed;
	numSeed++;
}


void Queue::OnBnClickedButton5()//ɾ����ѡ������
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int Selected_Index = m_SeedList.GetCurSel();
	if (Selected_Index >= 0)
	{
		m_SeedList.DeleteString(Selected_Index);
		/*����ʵ���ظ�����*/
	}
}


void Queue::OnBnClickedButton6()//�������������
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int Count = m_SeedList.GetCount();

	for (int i = Count; i >= 0; i--)
	{
		m_SeedList.DeleteString(i);
		numSeed--;
	}
}


static long zrng[] =
{ 1,
 1973272912, 281629770,  20006270,1280689831,2096730329,1933576050,
  913566091, 246780520,1363774876, 604901985,1511192140,1259851944,
  824064364, 150493284, 242708531,  75253171,1964472944,1202299975,
  233217322,1911216000, 726370533, 403498145, 993232223,1103205531,
  762430696,1922803170,1385516923,  76271663, 413682397, 726466604,
  336157058,1432650381,1120463904, 595778810, 877722890,1046574445,
   68911991,2088367019, 748545416, 622401386,2122378830, 640690903,
 1774806513,2132545692,2079249579,  78130110, 852776735,1187867272,
 1351423507,1645973084,1997049139, 922510944,2045512870, 898585771,
  243649545,1004818771, 773686062, 403188473, 372279877,1901633463,
  498067494,2087759558, 493157915, 597104727,1530940798,1814496276,
  536444882,1663153658, 855503735,  67784357,1432404475, 619691088,
  119025595, 880802310, 176192644,1116780070, 277854671,1366580350,
 1142483975,2026948561,1053920743, 786262391,1792203830,1494667770,
 1923011392,1433700034,1244184613,1147297105, 539712780,1545929719,
  190641742,1645390429, 264907697, 620389253,1502074852, 927711160,
  364849192,2049576050, 638580085, 547070247 };

void Queue::OnBnClickedButton7()//�����������
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	for (int i = 0; i < 101; i++)
	{
		lcgrandst(zrng[i], i);
	}
}


void Queue::OnBnClickedButton8()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_queue.DeleteAllItems();
}


void Queue::OnBnClickedButton9()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ShellExecute(NULL, _T("open"), _T("Report2.pdf"), NULL, NULL, SW_SHOWNORMAL);
}


