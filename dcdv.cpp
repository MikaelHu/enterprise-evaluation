#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "dcdv.h"


void test_dcdv_ev() {
	s_comp_info_t comp_info{ "AAA", 8.21, 186.0, 18.5, 72.0, 16.0 };
	s_dcfe_params_t dcfe_params{ 2020, 1, {{ -1, 0.03, 0.08, 0.0}} };
	//s_dcfe_params_t dcfe_params{ 2020, 4, {{ 3, 0.12, 0.09, 0.0}, { 4, 0.10, 0.09, 0.0}, { 5, 0.08, 0.09, 0.0}, { -1, 0.03, 0.08, 0.0}} };
	s_comp_ev_res_t comp_ev_res{ 0 };

	printf("###########DCDV企业估值############\n");
	printf("###########企业信息###############\n");
	comp_info.dump_info();
	printf("###########DCDV参数################\n");
	dcfe_params.dump_info();
	int ret = dcdv(comp_info, dcfe_params, comp_ev_res);

	printf("###########企业估值############\n");
	comp_ev_res.dump_info();
}

#ifdef _DEBUG
#define TEST
#endif //_DEBUG

int main(int argc, char* argv[]) {
#ifdef TEST
	test_dcdv_ev();
#else
	if (argc <= 7) {
		printf("##############DCDV Usage###############\n");
		printf("HuDcdv.exe sn ni fcfe cy st ys g ke ... sc\n");
		//printf("Like bellow:\n");
		//printf("HuDcdv.exe 8.21 15.0 15.0 2020 4 3 0.15 0.09 4 0.12 0.09 5 0.08 0.09 -1 0.03 0.08 1.0\n");
		printf("################End##################\n");

		return -1;
	}
	else {
		s_comp_info_t comp_info{ 0 };
		s_dcfe_params_t dcfe_params{ 0 };
		s_comp_ev_res_t comp_ev_res{ 0 };

		comp_info.share_nums = std::atof(argv[1]);
		comp_info.net_income = std::atof(argv[2]);
		comp_info.fcfe = std::atof(argv[3]);
		dcfe_params.start_year = std::atoi(argv[4]);
		dcfe_params.num_steps = std::atoi(argv[5]);
		for(int i = 0; i < dcfe_params.num_steps; i++)
			dcfe_params.v_s_step_params[i] = { std::atoi(argv[6 + i * 3]), std::atof(argv[7 + i * 3]),
			std::atof(argv[8 + i * 3]), 0.0 };
		double safe_coef = std::atof(argv[argc - 1]);

		printf("###########DCDV企业估值############\n");
		printf("###########企业信息###############\n");
		comp_info.dump_info();
		printf("###########DCDV参数################\n");
		dcfe_params.dump_info();
		int ret = dcdv(comp_info, dcfe_params, comp_ev_res, safe_coef);

		printf("###########企业估值############\n");
		comp_ev_res.dump_info();

		return 0;
	}
#endif	//TEST
}