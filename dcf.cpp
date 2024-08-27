#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "dcf.h"


void test_dcfe_ev() {
	s_comp_info_t comp_info{ "AAA", 8.21, 186.0, 18.5, 72.0 };
	s_dcfe_params_t dcfe_params{ 2020, 1, {{ -1, 0.03, 0.08, 0.0, 0.257}} };
	//s_dcfe_params_t dcfe_params{ 2020, 4, {{ 3, 0.12, 0.09, 0.0, 0.257}, { 4, 0.10, 0.09, 0.0, 0.257}, { 5, 0.08, 0.09, 0.0, 0.257}, { -1, 0.03, 0.08, 0.0, 0.257}} };
	s_comp_ev_res_t comp_ev_res{ 0 };

	printf("###########DCFE企业估值############\n");
	printf("###########企业信息###############\n");
	comp_info.dump_info();
	printf("###########DCFE参数################\n");
	dcfe_params.dump_info();
	int ret =  dcfe(comp_info, dcfe_params, comp_ev_res);

	printf("###########企业估值############\n");
	comp_ev_res.dump_info();
}

#ifdef _DEBUG
#define TEST
#endif //_DEBUG

int main(int argc, char* argv[]) {
#ifdef TEST
	test_dcfe_ev();
#else
	if (argc <= 7) {
		printf("##############DCFE Usage###############\n");
		printf("HuDcf.exe sn ni na cy st ys g r roe ys g r roe... sc\n");
		//printf("Like bellow:\n");
		//printf("HuDcf.exe 8.21 18.5 72.0 2020 4 3 0.12 0.09 0.257 4 0.12 0.09 0.257 5 0.08 0.09 0.257 -1 0.03 0.08 0.257 1.0\n");
		printf("################End##################\n");

		return -1;
	}
	else {
		s_comp_info_t comp_info{ 0 };
		s_dcfe_params_t dcfe_params{ 0 };
		s_comp_ev_res_t comp_ev_res{ 0 };

		comp_info.share_nums = std::atof(argv[1]);
		comp_info.net_income = std::atof(argv[2]);
		comp_info.net_assets = std::atof(argv[3]);
		dcfe_params.start_year = std::atoi(argv[4]);
		dcfe_params.num_steps = std::atoi(argv[5]);
		for(int i = 0; i < dcfe_params.num_steps; i++)
			dcfe_params.v_s_step_params[i] = { std::atoi(argv[6 + i * 4]), std::atof(argv[7 + i * 4]),
			std::atof(argv[8 + i * 4]), 0.0, std::atof(argv[9 + i * 4]) };
		double safe_coef = std::atof(argv[argc - 1]);

		printf("###########DCFE企业估值############\n");
		printf("###########企业信息###############\n");
		comp_info.dump_info();
		printf("###########DCFE参数################\n");
		dcfe_params.dump_info();
		int ret = dcfe(comp_info, dcfe_params, comp_ev_res, safe_coef);

		printf("###########企业估值############\n");
		comp_ev_res.dump_info();

		return 0;
	}
#endif	//TEST
}