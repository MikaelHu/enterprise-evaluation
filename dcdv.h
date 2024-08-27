#pragma once

#include <math.h>
#include <vector>
#include <numeric>
#include <functional>

typedef struct s_step_params {
	int	years;				//年数
	double	grow_rate;		//fcfe增长率
	double	ke;
	double	pre_y_fcfe;		//前一年的fcfe

	void dump_info() {
		printf("年数	FCFE增长率	ke	前一年的FCFE\n");
		printf("%d	%.2f	%.4f	%.2f\n", years, grow_rate, ke, pre_y_fcfe);
	}
}s_step_params_t;

#define	NUM_STEPS 8
typedef struct s_dcfe_params {
	int		start_year;		//起始年份
	int		num_steps;		//阶段数
	s_step_params_t	v_s_step_params[NUM_STEPS];	//阶段参数

	void dump_info() {
		printf("起始年份	阶段数\n");
		printf("%d	%d\n", start_year, num_steps);
		for (int i = 0; i < num_steps; i++)
			v_s_step_params[i].dump_info();
	}
}s_dcfe_params_t;

typedef struct s_comp_info {
	char	name[128];		//公司名
	double	share_nums;		//总股本数（亿）
	double	total_income;	//总营收（亿）
	double	net_income;		//净利润（亿）
	double	net_assets;		//净资产（亿）
	double	fcfe;			//股权资本自由现金流（亿）

	void dump_info() {
		printf("公司名	总股本数（亿）	总营收（亿）	净利润（亿）	净资产（亿）	FCFE（亿）\n");
		printf("%s	%.2f	%.2f	%.2f	%.2f	%.2f\n", 
			name, share_nums, total_income, net_income, net_assets, fcfe);
	}
}s_comp_info_t;

typedef struct s_comp_ev_res
{
	double			equity_value;		//权益价值（估值）
	double			share_price_y;		//股票价格（估计年限内，不含永续期）
	double			share_price_t;		//股票价格（含永续期）
	double			pe_y;
	double			pe_t;

	void dump_info() {
		printf("权益价值（亿）	每股价格Y（元）	每股价格T（元）	PE_Y	PE_T\n");
		printf("%.2f	%.2f	%.2f	%.2f	%.2f\n", equity_value, share_price_y, share_price_t, pe_y, pe_t);
	}
}s_comp_ev_res_t;

int dcdv(s_comp_info_t comp_info, s_dcfe_params_t dcf_params, s_comp_ev_res_t& comp_ev_res, double safe_coef = 0.8) {
	std::vector<double>		v_fcfe;
	std::vector<double>		v_dis_fcfe;
	v_fcfe.reserve(16);
	v_dis_fcfe.reserve(16);
	
	//计算各阶段部分折现FCFE
	for (int i = 0; i < dcf_params.num_steps - 1; i++) {
		if(i > 0)
			dcf_params.v_s_step_params[i].pre_y_fcfe = v_fcfe.back();
		else
			dcf_params.v_s_step_params[i].pre_y_fcfe = comp_info.fcfe;

		for (int j = 0; j < dcf_params.v_s_step_params[i].years; j++) {
			double fcfe = dcf_params.v_s_step_params[i].pre_y_fcfe * pow(dcf_params.v_s_step_params[i].grow_rate + 1, j + 1.0);
			v_fcfe.emplace_back(fcfe);
			double ke = pow(dcf_params.v_s_step_params[i].ke + 1, j + 1.0);
			for (int m = i - 1; m >= 0; m--) {
				ke *= pow(dcf_params.v_s_step_params[m].ke + 1, dcf_params.v_s_step_params[m].years);
			}
			double dis_fcfe = fcfe / ke;
			v_dis_fcfe.emplace_back(dis_fcfe);
		}
	}

	//计算永续部分折现FCFE
	double g = dcf_params.v_s_step_params[dcf_params.num_steps - 1].grow_rate;
	double r = dcf_params.v_s_step_params[dcf_params.num_steps - 1].ke;
	double fcfe_forever{ 0.0f };
	if(!v_fcfe.empty())
		fcfe_forever = v_fcfe.back() * (1 + g) / (r - g);
	else
		fcfe_forever = comp_info.fcfe * (1 + g) / (r - g);
	v_fcfe.emplace_back(fcfe_forever);

	double ke_forever{ 1.0f };
	for (int m = 0; m < dcf_params.num_steps - 1; m++) {
		ke_forever *= pow(dcf_params.v_s_step_params[m].ke + 1, dcf_params.v_s_step_params[m].years);
	}
	double dis_fcfe_forever = fcfe_forever / ke_forever;
	v_dis_fcfe.emplace_back(dis_fcfe_forever);

	comp_ev_res.equity_value = std::accumulate(v_dis_fcfe.begin(), v_dis_fcfe.end(), 0.0f);
	comp_ev_res.share_price_y = (comp_ev_res.equity_value - v_dis_fcfe.back()) / comp_info.share_nums;
	comp_ev_res.share_price_t = comp_ev_res.equity_value / comp_info.share_nums;
	comp_ev_res.pe_y = (comp_ev_res.equity_value - v_dis_fcfe.back()) / comp_info.net_income;
	comp_ev_res.pe_t = comp_ev_res.equity_value / comp_info.net_income;

	//打印各年份折现FCFE
	printf("#########各年份折现FCFE自由现金流############\n");
	printf("年份	FCFE(亿)	年份	FCFE(亿)\n");
	for (int i = 0; i < v_dis_fcfe.size(); i++) {
		if(i % 2 == 0 && i != 0)
			printf("\n%d	%.2f\t", dcf_params.start_year + i, v_dis_fcfe[i]);
		else
			printf("%d	%.2f\t", dcf_params.start_year + i, v_dis_fcfe[i]);
	}
	double ratio = dis_fcfe_forever / comp_ev_res.equity_value;
	printf("\n-----------------------------------\n");
	printf("折现FCFE各部分	价值百分比\n");
	printf("非永续部分：	%.3f\n", 1.0f - ratio);
	printf("永续部分  ：	%.3f\n", ratio);
	printf("\n");

	return 0;
}