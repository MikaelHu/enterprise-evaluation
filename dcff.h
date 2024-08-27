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
	double	wacc;
	double	kd;					//债务资本成本
	double	pre_y_ebit;			//前一年的息税前利润
	double	pre_y_debt;			//前一年的债务

	void dump_info() {
		printf("年数	ebit增长率	负债增长率	roic	wacc	kd	ebit(PY)	debt(PY)\n");
		printf("%d	%.2f	%.2f	%.4f	%.4f	%.4f	%.2f	%.2f\n", 
			years, grow_rate, grow_rate_debt, roic, wacc, kd, pre_y_ebit, pre_y_debt);
	}
}s_step_params_t;

#define	NUM_STEPS 8
typedef struct s_dcff_params {
	int		start_year;		//起始年份
	int		num_steps;		//阶段数
	s_step_params_t	v_s_step_params[NUM_STEPS];	//阶段参数

	void dump_info() {
		printf("起始年份	阶段数\n");
		printf("%d	%d\n", start_year, num_steps);
		for (int i = 0; i < num_steps; i++)
			v_s_step_params[i].dump_info();
	}
}s_dcff_params_t;

typedef struct s_comp_info {
	char	name[128];			//公司名
	double	share_nums;			//总股本数（亿）
	double	total_income;		//总营收（亿）
	double	net_income;			//净利润（亿）
	double	net_assets;			//净资产（亿）
	double	liability_ratio;	//负债率
	double	ebit;				//息税前利润
	double	tax_ratio;			//企业所得税率

	void dump_info() {
		printf("公司名	总股本数（亿）	总营收（亿）	净利润（亿）	净资产（亿)	负债率	EBIT	税率\n");
		printf("%s	%.2f	%.2f	%.2f	%.2f	%.3f	%.2f	%.2f\n",
			name, share_nums, total_income, net_income, net_assets, liability_ratio, ebit, tax_ratio);
	}
}s_comp_info_t;

typedef struct s_comp_ev_res
{
	double			enterpise_value;		//企业价值（估值）
	double			non_op_assets_value;	//非经营性资产价值
	double			debt_value;				//债务价值（估值）
	double			minority_value;			//少数股东价值
	double			equity_value;			//权益价值（估值）
	double			share_price;			//股票价格
	double			pe;

	void dump_info() {
		printf("企业价值	非经营性资产价值	债务价值	少数股东权益价值	权益价值	每股价格（元）	PE\n");
		printf("%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f\n", enterpise_value, non_op_assets_value, 
			debt_value, minority_value,	equity_value, share_price, pe);
	}
}s_comp_ev_res_t;

int dcff(s_comp_info_t comp_info, s_dcff_params_t dcff_params, s_comp_ev_res_t& comp_ev_res, double safe_coef = 0.8) {
	std::vector<double>		v_ebit;
	std::vector<double>		v_fcff;
	std::vector<double>		v_dis_fcff;
	std::vector<double>		v_debt;
	std::vector<double>		v_debt_fcf;
	std::vector<double>		v_dis_debt_fcf;
	v_ebit.reserve(16);
	v_fcff.reserve(16);
	v_dis_fcff.reserve(16);
	v_debt.reserve(16);
	v_debt_fcf.reserve(16);
	v_dis_debt_fcf.reserve(16);

	//计算各阶段部分折现FCFF
	for (int i = 0; i < dcff_params.num_steps - 1; i++) {
		if (i > 0)
			dcff_params.v_s_step_params[i].pre_y_ebit = v_ebit.back();
		else
			dcff_params.v_s_step_params[i].pre_y_ebit = comp_info.ebit;

		for (int j = 0; j < dcff_params.v_s_step_params[i].years; j++) {
			double ebit_s = dcff_params.v_s_step_params[i].pre_y_ebit * pow(dcff_params.v_s_step_params[i].grow_rate + 1, j + 1.0);
			v_ebit.emplace_back(ebit_s);
			double fcff = ebit_s * (1 - comp_info.tax_ratio) *(1 - dcff_params.v_s_step_params[i].grow_rate / dcff_params.v_s_step_params[i].roic);
			v_fcff.emplace_back(fcff);
			double wacc_s = pow(dcff_params.v_s_step_params[i].wacc + 1, j + 1.0);
			for (int m = i - 1; m >= 0; m--) {
				wacc_s *= pow(dcff_params.v_s_step_params[m].wacc + 1, dcff_params.v_s_step_params[m].years);
			}
			double dis_fcff_s = fcff / wacc_s;
			v_dis_fcff.emplace_back(dis_fcff_s);
		}
	}

	//计算各阶段折现债务价值
	for (int i = 0; i < dcff_params.num_steps - 1; i++) {
		if (i > 0)
			dcff_params.v_s_step_params[i].pre_y_debt = v_debt.back();
		else {
			double equity_multi = (1 - comp_info.liability_ratio) / comp_info.liability_ratio;
			dcff_params.v_s_step_params[i].pre_y_debt = comp_info.net_assets / equity_multi;
		}

		for (int j = 0; j < dcff_params.v_s_step_params[i].years; j++) {
			double debt_s = dcff_params.v_s_step_params[i].pre_y_debt * pow(dcff_params.v_s_step_params[i].grow_rate_debt + 1, j + 1.0);
			v_debt.emplace_back(debt_s);
			double debt_fcf_s = debt_s * dcff_params.v_s_step_params[i].kd * comp_info.tax_ratio;
			v_debt_fcf.emplace_back(debt_fcf_s);
			double kd_s = pow(dcff_params.v_s_step_params[i].kd + 1, j + 1.0);
			for (int m = i - 1; m >= 0; m--) {
				kd_s *= pow(dcff_params.v_s_step_params[m].kd + 1, dcff_params.v_s_step_params[m].years);
			}
			double dis_debt_fcf_s = debt_fcf_s / kd_s;
			v_dis_debt_fcf.emplace_back(dis_debt_fcf_s);
		}
	}

	//计算永续部分折现FCFF
	double g_f = dcff_params.v_s_step_params[dcff_params.num_steps - 1].grow_rate;
	double r_f = dcff_params.v_s_step_params[dcff_params.num_steps - 1].wacc;
	double ebit_forever1{ 0.0f };
	double fcff_forever1{ 0.0f };
	double fcff_forever{ 0.0f };
	if (!v_ebit.empty()) {
		ebit_forever1 = v_ebit.back() * (1 + g_f);
		fcff_forever1 = ebit_forever1 * (1 - comp_info.tax_ratio) * (1 - g_f / r_f);
		fcff_forever = fcff_forever1 / (r_f - g_f);
	}
	else {
		ebit_forever1 = comp_info.ebit * (1 + g_f);
		fcff_forever1 = ebit_forever1 * (1 - comp_info.tax_ratio) * (1 - g_f / r_f);
		fcff_forever = fcff_forever1 / (r_f - g_f);
	}
	v_fcff.emplace_back(fcff_forever);

	double wacc_forever{ 1.0f };
	for (int m = 0; m < dcff_params.num_steps - 1; m++) {
		wacc_forever *= pow(dcff_params.v_s_step_params[m].wacc + 1, dcff_params.v_s_step_params[m].years);
	}
	double dis_fcff_forever = fcff_forever / wacc_forever;
	v_dis_fcff.emplace_back(dis_fcff_forever);

	//计算永续部分折现债务价值
	double d_g = dcff_params.v_s_step_params[dcff_params.num_steps - 1].grow_rate_debt;
	double d_r = dcff_params.v_s_step_params[dcff_params.num_steps - 1].kd;
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
	double debt_forever_fcf = debt_forever * dcff_params.v_s_step_params[dcff_params.num_steps - 1].kd * comp_info.tax_ratio;
	v_debt_fcf.emplace_back(debt_forever_fcf);

	double kd_forever{ 1.0f };
	for (int m = 0; m < dcff_params.num_steps - 1; m++) {
		kd_forever *= pow(dcff_params.v_s_step_params[m].kd + 1, dcff_params.v_s_step_params[m].years);
	}
	double dis_debt_forever_fcf = debt_forever_fcf / kd_forever;
	v_dis_debt_fcf.emplace_back(dis_debt_forever_fcf);

	//计算估值结果
	comp_ev_res.enterpise_value = std::accumulate(v_dis_fcff.begin(), v_dis_fcff.end(), 0.0f);
	comp_ev_res.debt_value = std::accumulate(v_dis_debt_fcf.begin(), v_dis_debt_fcf.end(), 0.0f);
	comp_ev_res.equity_value = comp_ev_res.enterpise_value + comp_ev_res.non_op_assets_value -
		comp_ev_res.debt_value - comp_ev_res.minority_value;
	comp_ev_res.share_price = comp_ev_res.equity_value / comp_info.share_nums;
	comp_ev_res.pe = comp_ev_res.equity_value / comp_info.net_income;

	//打印各年份折现FCFF
	printf("#########各年份折现FCFF自由现金流############\n");
	printf("年份	FCFF(亿)	年份	FCFF(亿)\n");
	for (int i = 0; i < v_dis_fcff.size(); i++) {
		if (i % 2 == 0 && i != 0)
			printf("\n%d	%.2f\t", dcff_params.start_year + i, v_dis_fcff[i]);
		else
			printf("%d	%.2f\t", dcff_params.start_year + i, v_dis_fcff[i]);
	}
	double ratio = dis_fcff_forever / comp_ev_res.enterpise_value;
	printf("\n-----------------------------------\n");
	printf("折现FCFF各部分	价值百分比\n");
	printf("非永续部分：	%.3f\n", 1.0f - ratio);
	printf("永续部分  ：	%.3f\n", ratio);
	printf("\n");

	return 0;
}