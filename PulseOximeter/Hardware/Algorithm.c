#include "stm32f10x.h"                  // Device header
#include "Algorithm.h"

//常数
#define XPI			(3.1415926535897932384626433832795)
#define XENTRY		(100)
#define XINCL		(XPI/2/XENTRY)
#define PI			3.1415926535897932384626433832795028841971

//正弦值表
static const double SinTable[] = 
{
	0.00000000000000000  , 0.015707317311820675 , 0.031410759078128292 , 0.047106450709642665 , 0.062790519529313374 ,
	0.078459095727844944 , 0.094108313318514325 , 0.10973431109104528  , 0.12533323356430426  , 0.14090123193758267  ,
	0.15643446504023087  , 0.17192910027940955  , 0.18738131458572463  , 0.20278729535651249  , 0.21814324139654256  ,
	0.23344536385590542  , 0.24868988716485479  , 0.26387304996537292  , 0.27899110603922928  , 0.29404032523230400  ,
	0.30901699437494740  , 0.32391741819814940  , 0.33873792024529142  , 0.35347484377925714  , 0.36812455268467797  ,
	0.38268343236508978  , 0.39714789063478062  , 0.41151435860510882  , 0.42577929156507272  , 0.43993916985591514  ,
	0.45399049973954680  , 0.46792981426057340  , 0.48175367410171532  , 0.49545866843240760  , 0.50904141575037132  ,
	0.52249856471594880  , 0.53582679497899666  , 0.54902281799813180  , 0.56208337785213058  , 0.57500525204327857  ,
	0.58778525229247314  , 0.60042022532588402  , 0.61290705365297649  , 0.62524265633570519  , 0.63742398974868975  ,
	0.64944804833018377  , 0.66131186532365183  , 0.67301251350977331  , 0.68454710592868873  , 0.69591279659231442  ,
	0.70710678118654757  , 0.71812629776318881  , 0.72896862742141155  , 0.73963109497860968  , 0.75011106963045959  ,
	0.76040596560003104  , 0.77051324277578925  , 0.78043040733832969  , 0.79015501237569041  , 0.79968465848709058  ,
	0.80901699437494745  , 0.81814971742502351  , 0.82708057427456183  , 0.83580736136827027  , 0.84432792550201508  ,
	0.85264016435409218  , 0.86074202700394364  , 0.86863151443819120  , 0.87630668004386369  , 0.88376563008869347  ,
	0.89100652418836779  , 0.89802757576061565  , 0.90482705246601958  , 0.91140327663544529  , 0.91775462568398114  ,
	0.92387953251128674  , 0.92977648588825146  , 0.93544403082986738  , 0.94088076895422557  , 0.94608535882754530  ,
	0.95105651629515353  , 0.95579301479833012  , 0.96029368567694307  , 0.96455741845779808  , 0.96858316112863108  ,
	0.97236992039767667  , 0.97591676193874743  , 0.97922281062176575  , 0.98228725072868872  , 0.98510932615477398  ,
	0.98768834059513777  , 0.99002365771655754  , 0.99211470131447788  , 0.99396095545517971  , 0.99556196460308000  ,
	0.99691733373312796  , 0.99802672842827156  , 0.99888987496197001  , 0.99950656036573160  , 0.99987663248166059  ,
	1.00000000000000000  
};

/**
  * @brief	向下取整
  * @param	x: 输入值
  * @retval	向下取整值
  */
double Floor(double x)
{
	if(x < 0.0)
	{
		return (double)((int)x) - 1;
	}
    else
	{
		return (double)((int)x);
	}
}
	
/**
  * @brief	取余
  * @param	x: 被除数
  * @param	y: 除数
  * @retval	x/y的余数
  */
double Mod(double x, double y)
{
	double Temp;
	if(y == 0.0)
	{
		return 0.0;
	}
	Temp = Floor(x / y);
	Temp = x - Temp * y;
	if((x < 0.0 && y > 0.0) || (x > 0.0 && y < 0.0))
	{
		Temp = Temp - y;
	}
	return Temp;
}

/**
  * @brief	正弦函数
  * @param	x: 自变量
  * @retval	x的正弦值
  */
double Sin(double x)
{
	int Flag = 0;	//诱导公式导致符号变化的标志位
	int n;
    double dx, sx, cx;
	//sin(-x) = -sin(x)
    if(x < 0)
	{
        Flag = 1;
		x = -x;
	}
	//sin(x + 2*kpi)= sin(x)
    x = Mod(x, 2 * XPI);
	//sin(x - pi) = -sin(x)
    if(x > XPI)
	{
        Flag = !Flag;
		x -= XPI;
	}
	//sin(pi - x) = sin(x)，确保最终0 <= x <= pi/2
    if(x > XPI / 2)
	{
        x = XPI - x;
	}
	//微分
    n = (int)(x / XINCL);
    dx = x - n * XINCL;
    if(dx > XINCL / 2)
	{
        ++n;
		dx -= XINCL;
	}
    sx = SinTable[n];
    cx = SinTable[XENTRY - n];
	//近似展开
    x = sx + dx*cx - (dx*dx)*sx/2 - (dx*dx*dx)*cx/6 + (dx*dx*dx*dx)*sx/24;
     
    return Flag	? -x : x;
}

/**
  * @brief	余弦函数
  * @param	x: 自变量
  * @retval	x的余弦值
  */
double Cos(double x)
{
	return Sin(x + XPI / 2);
}

/**
  * @brief	开平方
  * @param	x: 输入值
  * @retval	√x
  */
int Sqrt(int x)
{
	uint32_t Rem = 0, Root = 0, Divisor = 0;
	uint16_t i;
	for(i=0;i<16;i++)
	{
		Root <<= 1;
		Rem = ((Rem << 2) + (x>>30));
		x <<= 2;
		Divisor = (Root << 1) + 1;
		if(Divisor <= Rem)
		{
			Rem -= Divisor;
			Root++;
		}
	}
	return Root;
}

/**
  * @brief	复数乘法
  * @param	x: 复数1
  * @param	y: 复数2
  * @retval	x*y
  */
Complex EE(Complex x, Complex y)
{
	Complex R;
	R.Real = x.Real * y.Real - x.Imag * y.Imag;
	R.Imag = x.Real * y.Imag + x.Imag * y.Real;
	return R;
}

/**
  * @brief	快速傅里叶变换
  * @param	*xin: 输入（输出）数据数组，个数与n相同，不足补0
  * @param	n: 数据个数，需为2的次方
  * @retval	无
  */
void FFT(Complex *xin, uint16_t n)
{
	int f, m, nv2, nm1, i, k, l, j = 0;
	int le, lei, ip;
	Complex u, w, t;
	
	//逆序
	nv2 = n / 2;
	nm1 = n - 1;
	for(i=0;i<nm1;i++)
	{
		if(i < j)
		{
			t = xin[j];
			xin[j] = xin[i];
			xin[i] = t;
		}
		k = nv2;
		while(k <= j)
		{
			j = j - k;
			k = k / 2;
		}
		j = j + k;
	}
	//蝶形运算
	{
		f = n;
		for(l=1;(f=f/2)!=1;l++);
		for(m=1;m<=l;m++)
		{
			le = 2 << (m - 1);
			lei = le / 2;
			u.Real = 1.0;
			u.Imag = 0.0;
			w.Real = Cos(PI / lei);
			w.Imag = -Sin(PI / lei);
			for(j=0;j<=lei-1;j++)
			{
				for(i=j;i<=n-1;i=i+le)
				{
					ip = i + lei;
					t = EE(xin[ip], u);
					xin[ip].Real = xin[i].Real - t.Real;
					xin[ip].Imag = xin[i].Imag - t.Imag;
					xin[i].Real = xin[i].Real + t.Real;
					xin[i].Imag = xin[i].Imag + t.Imag;
				}
				u = EE(u, w);
			}
		}
	}
}

/**
  * @brief	查找峰值索引
  * @param	*Data: 数据数组
  * @param	n: 数据个数
  * @param	Start: 起点索引
  * @retval	峰值索引
  */
int Find_MaxIndex(Complex *Data, uint16_t n, uint16_t Start)
{
	uint16_t i = Start;
	uint16_t MaxIndex = i;
	float Temp = Data[i].Real;
	for(i=Start;i<n;i++)
	{
		if(Temp < Data[i].Real)
		{
			Temp = Data[i].Real;
			MaxIndex = i;
		}
	}
	return MaxIndex;
}
