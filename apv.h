#pragma once

#include <math.h>
#include <vector>
#include <numeric>
#include <functional>

typedef struct s_step_params {
	int	years;					//年数
	double	grow_rate;			//ebit增长率
	double	grow_rate_debt;		//负债增长率
	double	roic;				//当前阶段roic
	double	ku;					//无负债企业资本成本
	double	kd;					//债务资本成本
	double	pre_y_ebit;			//前一年的息税前利润
	double	pre_y_debt;			//前一年的债务

	void dump_info() {
		printf("年数	ebit增长率	负债增长率	roic	ku	kd	ebit(PY)	debt(PY)\n");
		printf("%d	%.2f	%.2f	%.4f	%.4f	%.4f	%.2f	%.2f\n",
			years, grow_rate, grow_rate_debt, roic, ku, kd, pre_y_ebit, pre_y_debt);
	}
}s_step_params_t;

#define	NUM_STEPS 8
typedef struct s_apv_params {
	int		start_year;		//起始年份
	int		num_steps;		//阶段数
	s_step_params_t	v_s_step_params[NUM_STEPS];	//阶段参数

	void dump_info() {
		printf("起始年份	阶段数\n");
		printf("%d	%d\n", start_year, num_steps);
		for (int i = 0; i < num_steps; i++)
			v_s_step_params[i].dump_info();
	}
}s_apv_params_t;

typedef struct s_comp_info {
	char	name[128];			//公司名
	double	share_nums;			//总股本数（亿）
	double	total_income;		//总营收（亿）
	double	net_income;			//净利润（亿）
	double	net_assets;			//净资产（亿）
	double	liability_ratio;	//负债率
	double	ebit;				//息税前利润
	double	tax_ratio;			//企业所得税率
	double	bankrupt_prob;		//破产概率
	double	bankruptcost2value_ratio;	//破产成本占无债务时公司价值的比例;;

	void dump_info() {
		printf("公司名	总股本数（亿）	总营收（亿）	净利润（亿）	净资产（亿)\
	负债率	ebit（亿）	税率	破产概率	破产成本占比\n");
		printf("%s	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f\n",
			name, share_nums, total_income, net_income, net_assets, liability_ratio,
			ebit, tax_ratio, bankrupt_prob, bankruptcost2value_ratio);
	}
}s_comp_info_t;

typedef struct s_comp_ev_res
{
	double			non_debt_enterpise_value;	//无负债企业价值（估值）
	double			tax_value;					//税收效益（估值）
	double			bankrupt_cost;				//预期破产成本
	double			minority_value;				//少数股东价值
	double			non_op_assets_value;		//非经营性资产价值
	double			equity_value;				//权益价值（估值）
	double			share_price;				//股票价格
	double			pe;

	void dump_info() {
		printf("无负债企业价值	税收效益	预期破产成本	非经营性资产价值	少数股东权益价值\
	权益价值	每股价格（元）	PE\n");
		printf("%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f\n", 
			non_debt_enterpise_value, tax_value, bankrupt_cost, non_op_assets_value, 
			minority_value, equity_value, share_price, pe);
	}
}s_comp_ev_res_t;

int apv(s_comp_info_t comp_info, s_apv_params_t apv_params, s_comp_ev_res_t& comp_ev_res, double safe_coef = 0.8) {
	std::vector<double>		v_ebit;
	std::vector<double>		v_ufcff;
	std::vector<double>		v_dis_ufcff;
	std::vector<double>		v_debt;
	std::vector<double>		v_debt_fcf;
	std::vector<double>		v_dis_debt_fcf;
	v_ebit.reserve(16);
	v_ufcff.reserve(16);
	v_dis_ufcff.reserve(16);
	v_debt.reserve(16);
	v_debt_fcf.reserve(16);
	v_dis_debt_fcf.reserve(16);

	//计算各阶段部分折现uFCFF
	for (int i = 0; i < apv_params.num_steps - 1; i++) {
		if (i > 0)
			apv_params.v_s_step_params[i].pre_y_ebit = v_ebit.back();
		else
			apv_params.v_s_step_params[i].pre_y_ebit = comp_info.ebit;

		for (int j = 0; j < apv_params.v_s_step_params[i].years; j++) {
			double ebit_s = apv_params.v_s_step_params[i].pre_y_ebit * pow(apv_params.v_s_step_params[i].grow_rate + 1, j + 1.0);
			v_ebit.emplace_back(ebit_s);
			double ufcff_s = ebit_s * (1 - comp_info.tax_ratio) * (1 - apv_params.v_s_step_params[i].grow_rate / apv_params.v_s_step_params[i].roic);
			v_ufcff.emplace_back(ufcff_s);
			double ku_s = pow(apv_params.v_s_step_params[i].ku + 1, j + 1.0);
			for (int m = i - 1; m >= 0; m--) {
				ku_s *= pow(apv_params.v_s_step_params[m].ku + 1, apv_params.v_s_step_params[m].years);
			}
			double dis_ufcff_s = ufcff_s / ku_s;
			v_dis_ufcff.emplace_back(dis_ufcff_s);
		}
	}

	//计算各阶段折现债务价值
	for (int i = 0; i < apv_params.num_steps - 1; i++) {
		if (i > 0)
			apv_params.v_s_step_params[i].pre_y_debt = v_debt.back();
		else {
			double equity_multi = (1 - comp_info.liability_ratio) / comp_info.liability_ratio;
			apv_params.v_s_step_params[i].pre_y_debt = comp_info.net_assets / equity_multi;
		}

		for (int j = 0; j < apv_params.v_s_step_params[i].years; j++) {
			double debt_s = apv_params.v_s_step_params[i].pre_y_debt * pow(apv_params.v_s_step_params[i].grow_rate_debt + 1, j + 1.0);
			v_debt.emplace_back(debt_s);
			double debt_fcf_s = debt_s * apv_params.v_s_step_params[i].kd * comp_info.tax_ratio;
			v_debt_fcf.emplace_back(debt_fcf_s);
			double kd_s = pow(apv_params.v_s_step_params[i].kd + 1, j + 1.0);
			for (int m = i - 1; m >= 0; m--) {
				kd_s *= pow(apv_params.v_s_step_params[m].kd + 1, apv_params.v_s_step_params[m].years);
			}
			double dis_debt_fcf_s = debt_fcf_s / kd_s;
			v_dis_debt_fcf.emplace_back(dis_debt_fcf_s);
		}
	}

	//计算永续部分折现uFCFF
	double g_f = apv_params.v_s_step_params[apv_params.num_steps - 1].grow_rate;
	double r_f = apv_params.v_s_step_params[apv_params.num_steps - 1].ku;
	double ebit_forever1{ 0.0f };
	double ufcff_forever1{ 0.0f };
	double ufcff_forever{ 0.0f };
	if (!v_ebit.empty()) {
		ebit_forever1 = v_ebit.back() * (1 + g_f);
		ufcff_forever1 = ebit_forever1 * (1 - comp_info.tax_ratio) * (1 - g_f / r_f);
		ufcff_forever = ufcff_forever1 / (r_f - g_f);
	}
	else {
		ebit_forever1 = comp_info.ebit * (1 + g_f);
		ufcff_forever1 = ebit_forever1 * (1 - comp_info.tax_ratio) * (1 - g_f / r_f);
		ufcff_forever = ufcff_forever1 / (r_f - g_f);
	}
	v_ufcff.emplace_back(ufcff_forever);

	double ku_forever{ 1.0f };
	for (int m = 0; m < apv_params.num_steps - 1; m++) {
		ku_forever *= pow(apv_params.v_s_step_params[m].ku + 1, apv_params.v_s_step_params[m].years);
	}
	double dis_ufcff_forever = ufcff_forever / ku_forever;
	v_dis_ufcff.emplace_back(dis_ufcff_forever);

	//计算永续部分折现债务价值
	double d_g = apv_params.v_s_step_params[apv_params.num_steps - 1].grow_rate_debt;
	double d_r = apv_params.v_s_step_params[apv_params.num_steps - 1].kd;
	double debt_forever{ 0.0f };
	if (!v_debt.empty()) {
		debt_forever = v_debt.back() * (1 + d_g) / (d_r - d_g);
	}
	else {
		double equity_multi = (1 - comp_info.liability_ratio) / comp_info.liability_ratio;
		double	debt = comp_info.net_assets / equity_multi;
		debt_forever = debt * (1 + d_g) / (d_r - d_g);
	}

	v_debt.emplace_back(debt_forever);
	double debt_forever_fcf = debt_forever * apv_params.v_s_step_params[apv_params.num_steps - 1].kd * comp_info.tax_ratio;
	v_debt_fcf.emplace_back(debt_forever_fcf);

	double kd_forever{ 1.0f };
	for (int m = 0; m < apv_params.num_steps - 1; m++) {
		kd_forever *= pow(apv_params.v_s_step_params[m].kd + 1, apv_params.v_s_step_params[m].years);
	}
	double dis_debt_forever_fcf = debt_forever_fcf / kd_forever;
	v_dis_debt_fcf.emplace_back(dis_debt_forever_fcf);

	//计算估值结果
	comp_ev_res.non_debt_enterpise_value = std::accumulate(v_dis_ufcff.begin(), v_dis_ufcff.end(), 0.0f);
	comp_ev_res.tax_value = std::accumulate(v_dis_debt_fcf.begin(), v_dis_debt_fcf.end(), 0.0f);
	comp_ev_res.bankrupt_cost = comp_info.bankrupt_prob * comp_info.bankruptcost2value_ratio * comp_ev_res.non_debt_enterpise_value;
	comp_ev_res.equity_value = comp_ev_res.non_debt_enterpise_value + comp_ev_res.tax_value - comp_ev_res.bankrupt_cost + 
		comp_ev_res.non_op_assets_value - comp_ev_res.minority_value;
	comp_ev_res.share_price = comp_ev_res.equity_value / comp_info.share_nums;
	comp_ev_res.pe = comp_ev_res.equity_value / comp_info.net_income;

	//打印各年份折现FCFF
	printf("#########各年份折现uFCFF自由现金流############\n");
	printf("年份	uFCFF(亿)	年份	FCFF(亿)\n");
	for (int i = 0; i < v_dis_ufcff.size(); i++) {
		if (i % 2 == 0 && i != 0)
			printf("\n%d	%.2f\t", apv_params.start_year + i, v_dis_ufcff[i]);
		else
			printf("%d	%.2f\t", apv_params.start_year + i, v_dis_ufcff[i]);
	}
	double ratio = dis_ufcff_forever / comp_ev_res.non_debt_enterpise_value;
	printf("\n-----------------------------------\n");
	printf("折现uFCFF各部分	价值百分比\n");
	printf("非永续部分：	%.3f\n", 1.0f - ratio);
	printf("永续部分  ：	%.3f\n", ratio);
	printf("\n");

	return 0;
}