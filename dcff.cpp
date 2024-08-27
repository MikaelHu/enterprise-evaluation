#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "dcff.h"


void test_dcff_ev() {
	s_comp_info_t comp_info{ "AAA", 8.21, 186.0, 18.5, 72.0, 0.41, 21.7, 0.15 };
	s_dcff_params_t dcff_params{ 2020, 1, {{ -1, 0.03, 0.03, 0.258, 0.08, 0.056, 0.0, 0.0}} };
	/*s_dcff_params_t dcff_params{ 2020, 4, {{ 3, 0.12, 0.12, 0.258, 0.09, 0.056, 0.0, 0.0},
		{ 4, 0.10, 0.10, 0.258, 0.09, 0.056, 0.0, 0.0},
		{ 5, 0.08, 0.08, 0.258, 0.09, 0.056, 0.0, 0.0},
		{ -1, 0.03, 0.03, 0.258, 0.08, 0.056, 0.0, 0.0}} };*/
	s_comp_ev_res_t comp_ev_res{ 0 };

	printf("###########DCFF企业估值############\n");
	printf("###########企业信息###############\n");
	comp_info.dump_info();
	printf("###########DCFF参数################\n");
	dcff_params.dump_info();
	int ret = dcff(comp_info, dcff_params, comp_ev_res);

	printf("###########企业估值############\n");
	comp_ev_res.dump_info();
}

#ifdef _DEBUG
#define TEST
#endif //_DEBUG

int main(int argc, char* argv[]) {
#ifdef TEST
	test_dcff_ev();
#else
	if (argc <= 7) {
		printf("##############DCFF Usage###############\n");
		printf("HuDcff.exe sn ni na lr eb tr cy st ys g gd roic wacc kd ... sc\n");
		//printf("Like bellow:\n");
		//printf("HuDcff.exe 8.21 18.5 72.0 0.41 21.7 0.15 2020 2 1 0.10 0.10 0.258 0.09 0.056 -1 0.03 0.03 0.258 0.08 0.056 1.0\n");
		printf("################End##################\n");

		return -1;
	}
	else {
		s_comp_info_t comp_info{ 0 };
		s_dcff_params_t dcff_params{ 0 };
		s_comp_ev_res_t comp_ev_res{ 0 };

		comp_info.share_nums = std::atof(argv[1]);
		comp_info.net_income = std::atof(argv[2]);
		comp_info.net_assets = std::atof(argv[3]);
		comp_info.liability_ratio = std::atof(argv[4]);
		comp_info.ebit = std::atof(argv[5]);
		comp_info.tax_ratio = std::atof(argv[6]);
		dcff_params.start_year = std::atoi(argv[7]);
		dcff_params.num_steps = std::atoi(argv[8]);
		for (int i = 0; i < dcff_params.num_steps; i++)
			dcff_params.v_s_step_params[i] = { std::atoi(argv[9 + i * 6]), std::atof(argv[10 + i * 6]),
			std::atof(argv[11 + i * 6]), std::atof(argv[12 + i * 6]), std::atof(argv[13 + i * 6]), 
			std::atof(argv[14 + i * 6]), 0.0, 0.0 };
		double safe_coef = std::atof(argv[argc - 1]);

		printf("###########DCFF企业估值############\n");
		printf("###########企业信息###############\n");
		comp_info.dump_info();
		printf("###########DCFF参数################\n");
		dcff_params.dump_info();
		int ret = dcff(comp_info, dcff_params, comp_ev_res, safe_coef);

		printf("###########企业估值############\n");
		comp_ev_res.dump_info();

		return 0;
	}
#endif	//TEST
}