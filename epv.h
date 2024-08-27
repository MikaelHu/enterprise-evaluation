#pragma once

#include <math.h>
#include <vector>
#include <numeric>
#include <functional>

typedef struct s_step_params {
	int	years;				//年数
	double	grow_rate;		//权益增长率
	double	ke;				//权益成本
	//double	required_ret;	//折现率
	double	pre_y_eq;		//前一年的权益净值
	double	roe;			//权益回报率

	void dump_info() {
		printf("年数	g	ke	前一年的权益净值	roe\n");
		printf("%d	%.2f 	%.4f	%.2f	%.2f\n", years, grow_rate, ke, pre_y_eq, roe);
	}
}s_step_params_t;

#define	NUM_STEPS 8
typedef struct s_epv_params {
	int		start_year;		//起始年份
	int		num_steps;		//阶段数
	s_step_params_t	v_s_step_params[NUM_STEPS];	//阶段参数

	void dump_info() {
		printf("起始年份	阶段数\n");
		printf("%d	%d\n", start_year, num_steps);
		for (int i = 0; i < num_steps; i++)
			v_s_step_params[i].dump_info();
	}
}s_epv_params_t;

typedef struct s_comp_info {
	char	name[128];		//公司名
	double	share_nums;		//总股本数（亿）
	double	total_income;	//总营收（亿）
	double	net_income;		//净利润（亿）
	double	net_assets;		//净资产（亿）

	void dump_info() {
		printf("公司名	总股本数（亿）	总营收（亿）	净利润（亿）	净资产（亿）\n");
		printf("%s	%.2f	%.2f	%.2f	%.2f\n",
			name, share_nums, total_income, net_income, net_assets);
	}
}s_comp_info_t;

typedef struct s_comp_ev_res
{
	double			enterpise_value;	//权益价值（估值）
	double			share_price_y;		//股票价格（估计年限内，不含永续期）
	double			share_price_t;		//股票价格（含永续期）
	double			pe_y;
	double			pe_t;

	void dump_info() {
		printf("权益价值（亿）	每股价格Y（元）	每股价格T（元）	PE_Y	PE_T\n");
		printf("%.2f	%.2f	%.2f	%.2f	%.2f\n", enterpise_value, share_price_y, share_price_t, pe_y, pe_t);
	}
}s_comp_ev_res_t;

int epv(s_comp_info_t comp_info, s_epv_params_t epv_params, s_comp_ev_res_t& comp_ev_res, double safe_coef = 0.8) {
	std::vector<double>		v_eq;
	std::vector<double>		v_ep;
	std::vector<double>		v_dis_ep;
	v_eq.reserve(16);
	v_ep.reserve(16);
	v_dis_ep.reserve(16);

	//计算各阶段部分折现股权利润
	for (int i = 0; i < epv_params.num_steps - 1; i++) {
		if (i > 0)
			epv_params.v_s_step_params[i].pre_y_eq = v_eq.back();
		else
			epv_params.v_s_step_params[i].pre_y_eq = comp_info.net_assets;

		for (int j = 0; j < epv_params.v_s_step_params[i].years; j++) {
			double eq = epv_params.v_s_step_params[i].pre_y_eq * pow(epv_params.v_s_step_params[i].grow_rate + 1, j + 1.0);
			v_eq.emplace_back(eq);
			double ep = eq * (epv_params.v_s_step_params[i].roe - epv_params.v_s_step_params[i].ke);
			v_ep.emplace_back(ep);
			double ke_s = pow(epv_params.v_s_step_params[i].ke + 1, j + 1.0);
			for (int m = i - 1; m >= 0; m--) {
				ke_s *= pow(epv_params.v_s_step_params[m].ke + 1, epv_params.v_s_step_params[m].years);
			}
			double dis_ep = ep / ke_s;
			v_dis_ep.emplace_back(dis_ep);
		}
	}

	//计算永续部分折现股权利润
	double g = epv_params.v_s_step_params[epv_params.num_steps - 1].grow_rate;
	double r = epv_params.v_s_step_params[epv_params.num_steps - 1].ke;
	double eq_forever{ 0.0f };
	if(!v_eq.empty())
		eq_forever = v_eq.back() * (1 + g) / (r - g);
	else
		eq_forever = comp_info.net_assets * (1 + g) / (r - g);

	v_eq.emplace_back(eq_forever);
	double ep_forever = eq_forever * (epv_params.v_s_step_params[epv_params.num_steps - 1].roe - epv_params.v_s_step_params[epv_params.num_steps - 1].ke);
	v_ep.emplace_back(ep_forever);

	double ke_forever{ 1.0f };
	for (int m = 0; m < epv_params.num_steps - 1; m++) {
		ke_forever *= pow(epv_params.v_s_step_params[m].ke + 1, epv_params.v_s_step_params[m].years);
	}
	double dis_ep_forever = ep_forever / ke_forever;
	v_dis_ep.emplace_back(dis_ep_forever);

	double net_assert1 = comp_info.net_assets * (1 + g);
	comp_ev_res.enterpise_value = net_assert1 + std::accumulate(v_dis_ep.begin(), v_dis_ep.end(), 0.0f);
	comp_ev_res.share_price_y = (comp_ev_res.enterpise_value - v_dis_ep.back()) / comp_info.share_nums;
	comp_ev_res.share_price_t = comp_ev_res.enterpise_value / comp_info.share_nums;
	comp_ev_res.pe_y = (comp_ev_res.enterpise_value - v_dis_ep.back()) / comp_info.net_income;
	comp_ev_res.pe_t = comp_ev_res.enterpise_value / comp_info.net_income;

	//打印各年份折现股权利润
	printf("#########各年份折现股权利润############\n");
	printf("年份	EP(亿)	年份	EP(亿)\n");
	for (int i = 0; i < v_dis_ep.size(); i++) {
		if (i % 2 == 0 && i != 0)
			printf("\n%d	%.2f\t", epv_params.start_year + i, v_dis_ep[i]);
		else
			printf("%d	%.2f\t", epv_params.start_year + i, v_dis_ep[i]);
	}
	double ratio = dis_ep_forever / (comp_ev_res.enterpise_value - comp_info.net_assets);
	printf("\n-----------------------------------\n");
	printf("折现股权利润各部分	价值百分比\n");
	printf("非永续部分：	%.3f\n", 1.0f - ratio);
	printf("永续部分  ：	%.3f\n", ratio);
	printf("\n");

	return 0;
}