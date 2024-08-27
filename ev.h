#pragma once
#include <math.h>

typedef struct s_comp_info {
	char	name[128]{ "" };			//公司名
	double	share_nums{ 0.0f };			//总股本数（亿）
	double	total_income{ 0.0f };		//总营收（亿）
	double	income_inc{ 0.0f };			//总营收增长率
	double	net_income{ 0.0f };			//净利润（亿）
	double	n_income_inc{ 0.0f };		//净利润增长率
	double	net_assets{ 0.0f };			//净资产（亿）
	double	roe{ 0.0f };				//净资产收益率
	double	fcf{ 0.0f };				//自由现金流（亿）

	void dump_info() {
		printf("公司名	总股本数（亿）	总营收（亿）	总营收增长率	净利润（亿）	净利润增长率	净资产（亿）	ROE	自由现金流（亿）\n");
		printf("%s	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f\n",
			name, share_nums, total_income, income_inc, net_income, n_income_inc, 
			net_assets, roe, fcf);
	}
}s_comp_info_t;

typedef struct s_ev_params {
	int years{ 3 };
	double r{ 0.09f };			//折现率
	double inf{ 0.00f };
	double alpha{ 1.0f };		//企业价值倍数
	double beta{ 1.0f };		//ROE调整系数
	double safe_coef{ 1.0f };	//安全系数

	void dump_info() {
		printf("年限	r	inf	alpha	beta	safe_coef\n");
		printf("%d	%.4f	%.4f	%.2f	%.2f	%.2f\n", 
			years, r, inf, alpha, beta, safe_coef);
	}
}s_ev_params_t;

typedef struct s_comp_ev_res
{
	double			ev;		//企业价值（估值）
	double			sp;		//股票价格
	double			pe;
	double			peg;

	void dump_info() {
		printf("企业价值（亿）	每股价格（元）	PE	PEG\n");
		printf("%.2f	%.2f	%.2f	%.2f\n", ev, sp, pe, peg);
	}
}s_comp_ev_res_t;

constexpr double alpha1{ 0.9f };
constexpr double beta1{ 3.0f };
constexpr double gamma{ 1.0f };
constexpr double sigma{ 0.5f };

double calc_pe_y(int year, double r) {
	return /*pow(1.0f + r, year)*/ 1.0f / r;
}
double calc_pe(double roe, double r) {
	return pow(2 * r * roe * 100, 0.5) / r;
}
double calc_pe_g(double roe, double g, double r) {
	double f_g{ 0.0 };
	double f_g_roe{ 0.0 };

	f_g = alpha1 * exp(beta1 * g);
	f_g_roe = pow(gamma * roe / r, f_g * sigma);
	return f_g_roe / r;
}
double calc_pe_b(double roe, double beta, double r) {
	return roe * beta / pow(r, 2);
}
double calc_grow_alpha(double r, double g, int year, double alpha) {
	return pow((1 + g) / (1 + r), year) * alpha;
}
double calc_ev_y(double eco_income, double r, double g, int year,
	double inf, double alpha, double beta, double safe_coef) {
	double pe = calc_pe_y(year, r - inf);
	double grow_alpha = calc_grow_alpha(r - inf, g, year, alpha);
	double ev = eco_income * pe * grow_alpha * safe_coef;

	return ev;
}
double calc_ev(double eco_income, double roe, double r, double g, int year,
	double inf, double alpha, double beta, double safe_coef) {
	double pe = calc_pe(roe, r - inf);
	double grow_alpha = calc_grow_alpha(r - inf, g, year, alpha);
	double ev = eco_income * pe * grow_alpha * safe_coef;

	return ev;
}
double calc_ev_g(double eco_income, double roe, double r, double g, int year,
	double inf, double alpha, double beta, double safe_coef) {
	double pe = calc_pe_g(roe, g, r - inf);
	//double grow_alpha = calc_grow_alpha(r - inf, g, year, alpha);
	double ev = eco_income * pe /** grow_alpha*/ * alpha * safe_coef;

	return ev;
}
double calc_ev_b(double eco_income, double roe, double r, double g, int year,
	double inf, double alpha, double beta, double safe_coef) {
	double pe = calc_pe_b(roe, beta, r - inf);
	double grow_alpha = calc_grow_alpha(r - inf, g, year, alpha);
	double ev = eco_income * pe * grow_alpha * safe_coef;

	return ev;
}

//int ev(double eco_income,
//	double roe,
//	double share_num,			//股本数
//	double g,
//	int year,
//	double r = 0.09f,			//折现率
//	double inf = 0.03f,
//	double alpha = 1.0f,			//企业价值倍数
//	double beta = 1.0f,			//ROE修正系数
//	double safe_coef = 1.0f		//安全系数
//) {
int ev(s_comp_info_t comp_info, s_ev_params_t ev_params, 
	s_comp_ev_res_t& comp_ev_res_y, s_comp_ev_res_t& comp_ev_res, 
	s_comp_ev_res_t& comp_ev_res_g,	s_comp_ev_res_t& comp_ev_res_b){
	printf("#################################################\n");
	printf("#######################企业估值##################\n");
	printf("#################################################\n\n");
	printf("-----------------------估值参数---------------------------\n");
	//printf("r	inf	alpha	beta	safe_coef\n");
	//printf("%0.2f	%0.2f	%0.2f	%0.2f	%0.2f\n", r, inf, alpha, beta, safe_coef);
	ev_params.dump_info();

	//printf("##########calc_pe_g超参数##########\n");
	//printf("alpha1	beta1	gamma	sigma\n");
	//printf("%.2f	%.2f	%.2f	%.2f	%.2f\n", alpha1, beta1, gamma, sigma);

	printf("-----------------------企业信息---------------------------\n");
	//printf("eco_income(亿)	股本数(亿)	roe	g	years\n");
	//printf("%0.2f		%0.2f	%0.2f	%0.2f	%d\n", eco_income, share_num, roe, g, year);
	comp_info.dump_info();

	//ev_y
	comp_ev_res_y.ev = calc_ev_y(comp_info.net_income, ev_params.r,
		comp_info.n_income_inc, ev_params.years, ev_params.inf, ev_params.alpha,
		ev_params.beta, ev_params.safe_coef);
	comp_ev_res_y.sp = comp_ev_res_y.ev / comp_info.share_nums;
	comp_ev_res_y.pe = comp_ev_res_y.ev / comp_info.net_income;
	comp_ev_res_y.peg = comp_ev_res_y.pe / (comp_info.n_income_inc * 100);

	//ev
	comp_ev_res.ev = calc_ev(comp_info.net_income, comp_info.roe, ev_params.r,
		comp_info.n_income_inc, ev_params.years, ev_params.inf, ev_params.alpha, 
		ev_params.beta, ev_params.safe_coef);
	comp_ev_res.sp = comp_ev_res.ev / comp_info.share_nums;
	comp_ev_res.pe = comp_ev_res.ev / comp_info.net_income;
	comp_ev_res.peg = comp_ev_res.pe / (comp_info.n_income_inc * 100);

	//ev_g
	comp_ev_res_g.ev = calc_ev_g(comp_info.net_income, comp_info.roe, ev_params.r,
		comp_info.n_income_inc, ev_params.years, ev_params.inf, ev_params.alpha, 
		ev_params.beta, ev_params.safe_coef);
	comp_ev_res_g.sp = comp_ev_res_g.ev / comp_info.share_nums;
	comp_ev_res_g.pe = comp_ev_res_g.ev / comp_info.net_income;
	comp_ev_res_g.peg = comp_ev_res_g.pe / (comp_info.n_income_inc * 100);

	//ev_b
	comp_ev_res_b.ev = calc_ev_b(comp_info.net_income, comp_info.roe, ev_params.r,
		comp_info.n_income_inc, ev_params.years, ev_params.inf, ev_params.alpha, 
		ev_params.beta, ev_params.safe_coef);
	comp_ev_res_b.sp = comp_ev_res_b.ev / comp_info.share_nums;
	comp_ev_res_b.pe = comp_ev_res_b.ev / comp_info.net_income;
	comp_ev_res_b.peg = comp_ev_res_b.pe / (comp_info.n_income_inc * 100);

	printf("-----------------------估值结果---------------------------\n");
	//printf("ev(亿)	ev_1(亿)	price(￥)	price1(￥)	pe	pe_b	peg	peg_b\n");
	//printf("%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f\n",
	//	ev, ev_b, price, price_b, pe, pe_b, peg, peg_b);
	printf("-----------------------EV_Y-------------------------------\n");
	comp_ev_res_y.dump_info();
	printf("-----------------------EV---------------------------------\n");
	comp_ev_res.dump_info();
	printf("-----------------------EV_G-------------------------------\n");
	comp_ev_res_g.dump_info();
	printf("-----------------------EV_B-------------------------------\n");
	comp_ev_res_b.dump_info();
	printf("----------------------------------------------------------\n");

	return 0;
}
