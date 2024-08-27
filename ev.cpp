#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "ev.h"


void test_calc_pe_y(){
	printf("###Test calc pe_y###\n");
	printf("###pe_y VS year###\n");
	printf("year	r	pe_y\n");

	double r = 0.08f;
	int year = 3;
	double ret = calc_pe_y(year, r);
	printf("%d	%0.4f	%0.2f\n", year, r, ret);
}

void test_calc_pe() {
	printf("###Test calc pe###\n");
	printf("###pe VS roe###\n");
	printf("roe	r	pe\n");

	double r = 0.08f;
	for (int i = 0; i < 16; i++) {
		double roe = 3.0 * i/100;
		double ret = calc_pe(roe, r);
		printf("%0.2f	%0.4f	%0.2f\n", roe, r, ret);
	}
}
void test_calc_pe_g() {
	printf("###Test calc pe_g###\n");
	printf("##########calc_pe_g超参数##########\n");
	printf("alpha1	beta1	gamma	sigma\n");
	printf("%.2f	%.2f	%.2f	%.2f\n", alpha1, beta1, gamma, sigma);
	printf("###pe_g VS roe###\n");
	printf("roe	g	r	pe_g\n");

	double r = 0.08f;
	double g = 0.00f;
	for (int i = 0; i < 16; i++) {
		double roe = 3.0 * i / 100;
		double ret = calc_pe_g(roe, g, r);
		printf("%0.2f	%0.2f	%0.4f	%0.2f\n", roe, g, r, ret);
	}

	printf("###pe_g VS g###\n");
	for (int j = 0; j < 10; j++) {
		double roe = j * 0.05;
		printf("roe	g	r	pe_g\n");
		for (int i = 0; i < 16; i++) {
			double g = 3.0 * i / 100;
			double ret = calc_pe_g(roe, g, r);
			printf("%0.2f	%0.2f	%0.4f	%0.2f\n", roe, g, r, ret);
		}
	}
}
void test_calc_pe_b() {
	printf("###Test calc pe_b###\n");
	printf("###pe_b VS roe###\n");
	printf("roe	r	beta	pe_b\n");

	double r = 0.08f;
	double beta = 1.0f;
	for (int i = 0; i < 16; i++) {
		double roe = 3.0 * i / 100;
		double ret = calc_pe_b(roe, beta, r);
		printf("%0.2f	%0.4f	%0.2f	%0.2f\n", roe, r, beta, ret);
	}
}

void test_calc_grow_alpha() {
	int year = 3;
	double r = 0.08f;
	double alpha = 1.0f;

	printf("###Test calc grow_alpha pe	peg###\n");
	printf("###grow_alpha VS g###\n");
	printf("g	r	year	alpha	grow_alpha	pe	peg\n");
	for (int i = 0; i < 16; i++) {
		double g = i * 0.03;
		double grow_alpha = calc_grow_alpha(r, g, year, alpha);
		double pe = grow_alpha / r;
		double peg = pe / (g * 100);
		printf("%0.2f	%0.4f	%d	%0.2f	%0.2f		%0.2f	%0.2f\n", g, r, year, alpha, grow_alpha, pe, peg);
	}
}

void test_calc_ev() {
	double eco_income = 20.0f;
	double r = 0.08f;
	int year = 3;
	double inf = 0.00f;
	double alpha = 1.0f;
	double beta = 1.0f;
	double safe_coef = 1.0f;

	printf("###Test calc ev###\n");
	printf("###估值参数###\n");
	printf("year	r	inf	alpha	beta	safe_coef\n");
	printf("%d	%0.4f	%0.4f	%0.2f	%0.2f	%0.2f\n\n", year, r, inf, alpha, beta, safe_coef);

	for (int j = 0; j < 8; j++) {
		printf("###ev VS roe###\n");
		printf("eco_income	roe	g	ev_y	ev	ev_g	ev_b	pe_y	pe	pe_g	pe_b	peg_y	peg	peg_g	peg_b\n");
		double g = /*0.05 +*/ 0.05 * j;
		for (int i = 0; i < 15; i++) {
			double roe = (3.0 + 3.0 * i) / 100;
			double ev_y = calc_ev_y(eco_income, r, g, year, inf, alpha, beta, safe_coef);
			double ev = calc_ev(eco_income, roe, r, g, year, inf, alpha, beta, safe_coef);
			double ev_g = calc_ev_g(eco_income, roe, r, g, year, inf, alpha, beta, safe_coef);
			double ev_b = calc_ev_b(eco_income, roe, r, g, year, inf, alpha, beta, safe_coef);
			double pe_y = ev_y / eco_income;
			double peg_y = pe_y / (g * 100);
			double pe = ev / eco_income;
			double peg = pe / (g * 100);
			double pe_g = ev_g / eco_income;
			double peg_g = pe_g / (g * 100);
			double pe_b = ev_b / eco_income;
			double peg_b = pe_b / (g * 100);
			printf("%0.2f		%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f\n", 
				eco_income, roe, g, ev_y, ev, ev_g, ev_b, pe_y, pe, pe_g, pe_b, peg_y, peg, peg_g, peg_b);
		}
	}

	printf("\n###估值参数###\n");
	printf("year	r	inf	alpha	beta	safe_coef\n");
	printf("%d	%0.4f	%0.4f	%0.2f	%0.2f	%0.2f\n", year, r, inf, alpha, beta, safe_coef);

	for (int j = 0; j < 8; j++) {
		printf("###ev VS g###\n");
		printf("eco_income	roe	g	ev_y	ev	ev_g	ev_b	pe_y	pe	pe_g	pe_b	peg_y	peg	peg_g	peg_b\n");
		double roe = /*0.05 + */0.05 * j;
		for (int i = 0; i < 15; i++) {	
			double g = i * 0.03;
			double ev_y = calc_ev_y(eco_income, r, g, year, inf, alpha, beta, safe_coef);
			double ev = calc_ev(eco_income, roe, r, g, year, inf, alpha, beta, safe_coef);
			double ev_g = calc_ev_g(eco_income, roe, r, g, year, inf, alpha, beta, safe_coef);
			double ev_b = calc_ev_b(eco_income, roe, r, g, year, inf, alpha, beta, safe_coef);
			double pe_y = ev_y / eco_income;
			double peg_y = pe_y / (g * 100);
			double pe = ev / eco_income;
			double peg = pe / (g * 100);
			double pe_g = ev_g / eco_income;
			double peg_g = pe_g / (g * 100);
			double pe_b = ev_b / eco_income;
			double peg_b = pe_b / (g * 100);
			printf("%0.2f		%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f	%0.2f\n",
				eco_income, roe, g, ev_y, ev, ev_g, ev_b, pe_y, pe, pe_g, pe_b, peg_y, peg, peg_g, peg_b);
		}
	}
}

//#define TEST_ALG
#ifndef TEST_ALG
#ifdef _DEBUG
#define TEST
#endif //_DEBUG

int main(int argc, char* argv[]) {
#ifdef TEST
	test_calc_pe_y();
	test_calc_pe();
	test_calc_pe_g();
	test_calc_pe_b();
	test_calc_grow_alpha();
	test_calc_ev();
#else
	if (argc < 6) {
		printf("##############EV Usage###############\n");
		//printf("HuEV.exe eco_income roe share_num g year r inf alpha beta safe_coef\n");
		printf("HuEV.exe ni roe sn g y r inf alpha beta sc\n");
		printf("The last five parameters have default values.\n");
		//printf("Like bellow:\n");
		//printf("HuEV.exe 15.0 0.25 8.21 0.12 3 0.09 0.01 1.0 0.8 1.0\n");
		printf("################End##################\n");
		
		return -1;
	}
	else {
		//double eco_income = 76.0f;
		//double roe = 0.27f;
		//double share_num = 60.83f;	//总股本数（亿）
		//double g = 0.09f;			//增长率
		//int year = 3;
		//double r = 0.09f;			//折现率
		//double inf = 0.03f;
		//double alpha = 1.0f;			//企业价值倍数
		//double beta = 1.0f;			//ROE修正系数
		//double safe_coef = 1.0f;		//安全系数

		s_comp_info_t comp_info;
		s_ev_params_t ev_params;
		s_comp_ev_res_t comp_ev_res_y, comp_ev_res, comp_ev_res_g, comp_ev_res_b;

		comp_info.net_income = std::atof(argv[1]);
		comp_info.roe = std::atof(argv[2]);
		comp_info.share_nums = std::atof(argv[3]);
		comp_info.n_income_inc = std::atof(argv[4]);
		ev_params.years = std::atoi(argv[5]);
		
		int count = 6;
		if(argc - count++ > 0)
			ev_params.r = std::atof(argv[6]);
		if (argc - count++ > 0)
			ev_params.inf = std::atof(argv[7]);
		if (argc - count++ > 0)
			ev_params.alpha = std::atof(argv[8]);
		if (argc - count++ > 0)
			ev_params.beta = std::atof(argv[9]);
		if (argc - count++ > 0)
			ev_params.safe_coef = std::atof(argv[10]);

		int ret = ev(comp_info, ev_params, 
			comp_ev_res_y, comp_ev_res, comp_ev_res_g, comp_ev_res_b);
		return ret;
	}
#endif //TEST
}

#else
int test_f_pe_roe_g() {
	double alpha{ 0.9f };
	double beta{ 3.0f };
	double g{ 0.0f };
	double roe{ 0.0f };
	double r{ 0.09f };
	double f_g{ 0.0f };
	double f_g_roe{ 0.0f };
	double gamma{ 1.0f };
	double sigma{ 0.5f };
	double pe{ 0.0f };

	printf("##########超参数##########\n");
	printf("alpha	beta	gamma	sigma\n");
	printf("%.2f	%.2f	%.2f	%.2f\n", alpha, beta, gamma, sigma);

	printf("##########f_g-alpha##########\n");
	printf("g	alpha	beta	f_g\n");
	for (int i = 0; i < 10; i++) {
		g = 0.09f;
		alpha = 0.76f + 0.02 * i;
		f_g = alpha * exp(beta * g);
		printf("%.2f	%.2f	%.2f	%.2f\n", g, alpha, beta, f_g);
	}
	printf("##########f_g-beta##########\n");
	printf("g	alpha	beta	f_g\n");
	for (int i = 0; i < 10; i++) {
		g = 0.09f;
		alpha = 0.9f;
		beta = 2.0f + 0.2 * i;
		f_g = alpha * exp(beta * g);
		printf("%.2f	%.2f	%.2f	%.2f\n", g, alpha, beta, f_g);
	}

	alpha = 0.9f;
	beta = 3.0f;
	printf("##########超参数##########\n");
	printf("alpha	beta	gamma	sigma\n");
	printf("%.2f	%.2f	%.2f	%.2f\n", alpha, beta, gamma, sigma);
	printf("##########roe##########\n");
	for (int j = 0; j < 10; j++) {
		roe = 0.05f + 0.01 * j;
		printf("##########g##########\n");
		printf("g	roe	r	gamma*roe/r	f_g	f_g_roe	pe\n");
		for (int i = 0; i < 10; i++) {
			g = 0.05 * i;
			f_g = alpha * exp(beta * g);
			f_g_roe = pow(gamma * roe / r, f_g * sigma);
			pe = f_g_roe / r;
			printf("%.2f	%.2f	%.4f	%.2f		%.2f	%.2f	%.2f\n",
				g, roe, r, gamma * roe / r, f_g, f_g_roe, pe);
		}
	}

	printf("##########超参数##########\n");
	printf("alpha	beta	gamma	sigma\n");
	printf("%.2f	%.2f	%.2f	%.2f\n", alpha, beta, gamma, sigma);
	printf("##########g##########\n");
	for (int j = 0; j < 10; j++) {
		g = 0.05 * j;
		printf("##########roe##########\n");
		printf("g	roe	r	gamma*roe/r	f_g	f_g_roe	pe\n");
		for (int i = 0; i < 10; i++) {
			roe = 0.05 * i;
			f_g = alpha * exp(beta * g);
			f_g_roe = pow(gamma * roe / r, f_g * sigma);
			pe = f_g_roe / r;
			printf("%.2f	%.2f	%.4f	%.2f		%.2f	%.2f	%.2f\n", 
				g, roe, r, gamma * roe / r, f_g, f_g_roe, pe);
		}
	}
	return 0;
}

int main(int argc, char* argv[]) {
	int ret = test_f_pe_roe_g();

	return 0;
}

#endif	//TEST_ALG