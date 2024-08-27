#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "apv.h"


void test_apv_ev() {
	s_comp_info_t comp_info{ "AAA", 8.21, 186.0, 18.5, 72.0, 0.41, 21.7, 0.15, 0.12, 0.20 };
	s_apv_params_t apv_params{ 2020, 1, {{-1, 0.03, 0.03, 0.258, 0.08, 0.056, 0.0, 0.0}} };
	/*s_apv_params_t apv_params{ 2020, 4, {{ 3, 0.12, 0.12, 0.258, 0.09, 0.056, 0.0, 0.0},
		{ 4, 0.10, 0.10, 0.258, 0.09, 0.056, 0.0, 0.0},
		{ 5, 0.08, 0.08, 0.258, 0.09, 0.056, 0.0, 0.0},
		{ -1, 0.03, 0.03, 0.258, 0.08, 0.056, 0.0, 0.0}} };*/
	s_comp_ev_res_t comp_ev_res{ 0 };

	printf("###########APV企业估值############\n");
	printf("###########企业信息###############\n");
	comp_info.dump_info();
	printf("###########APV参数################\n");
	apv_params.dump_info();
	int ret = apv(comp_info, apv_params, comp_ev_res);

	printf("###########企业估值############\n");
	comp_ev_res.dump_info();
}

#ifdef _DEBUG
#define TEST
#endif //_DEBUG

int main(int argc, char* argv[]) {
#ifdef TEST
	test_apv_ev();
#else
	if (argc <= 7) {
		printf("##############APV Usage###############\n");		
		printf("HuAPV.exe sn ni na lr eb tr brp brcvr cy st ys g gd roic ku kd ... sc\n");
		//printf("Like bellow:\n");
		//printf("HuAPV.exe 8.21 18.5 72.0 0.41 21.7 0.15 0.12 0.20 2020 2 1 0.10 0.10 0.258 0.09 0.056 -1 0.03 0.03 0.258 0.08 0.056 1.0\n");
		printf("################End##################\n");

		return -1;
	}
	else {
		s_comp_info_t comp_info{ 0 };
		s_apv_params_t apv_params{ 0 };
		s_comp_ev_res_t comp_ev_res{ 0 };

		comp_info.share_nums = std::atof(argv[1]);
		comp_info.net_income = std::atof(argv[2]);
		comp_info.net_assets = std::atof(argv[3]);
		comp_info.liability_ratio = std::atof(argv[4]);
		comp_info.ebit = std::atof(argv[5]);
		comp_info.tax_ratio = std::atof(argv[6]);
		comp_info.bankrupt_prob = std::atof(argv[7]);
		comp_info.bankruptcost2value_ratio = std::atof(argv[8]);
		apv_params.start_year = std::atoi(argv[9]);
		apv_params.num_steps = std::atoi(argv[10]);
		for (int i = 0; i < apv_params.num_steps; i++)
			apv_params.v_s_step_params[i] = { std::atoi(argv[11 + i * 6]), std::atof(argv[12 + i * 6]),
			std::atof(argv[13 + i * 6]), std::atof(argv[14 + i * 6]), std::atof(argv[15 + i * 6]),
			std::atof(argv[16 + i * 6]), 0.0, 0.0 };
		double safe_coef = std::atof(argv[argc - 1]);

		printf("###########APV企业估值############\n");
		printf("###########企业信息###############\n");
		comp_info.dump_info();
		printf("###########DCFF参数################\n");
		apv_params.dump_info();
		int ret = apv(comp_info, apv_params, comp_ev_res, safe_coef);

		printf("###########企业估值############\n");
		comp_ev_res.dump_info();

		return 0;
	}
#endif	//TEST
}