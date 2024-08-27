#pragma once

#include <math.h>
#include <vector>
#include <numeric>
#include <functional>

typedef struct s_step_params {
	int	years;					//����
	double	grow_rate;			//ebit������
	double	grow_rate_debt;		//��ծ������
	double	roic;				//��ǰ�׶�roic
	double	wacc;
	double	kd;					//ծ���ʱ��ɱ�
	double	pre_y_ebit;			//ǰһ���Ϣ˰ǰ����
	double	pre_y_debt;			//ǰһ���ծ��

	void dump_info() {
		printf("����	ebit������	��ծ������	roic	wacc	kd	ebit(PY)	debt(PY)\n");
		printf("%d	%.2f	%.2f	%.4f	%.4f	%.4f	%.2f	%.2f\n", 
			years, grow_rate, grow_rate_debt, roic, wacc, kd, pre_y_ebit, pre_y_debt);
	}
}s_step_params_t;

#define	NUM_STEPS 8
typedef struct s_dcff_params {
	int		start_year;		//��ʼ���
	int		num_steps;		//�׶���
	s_step_params_t	v_s_step_params[NUM_STEPS];	//�׶β���

	void dump_info() {
		printf("��ʼ���	�׶���\n");
		printf("%d	%d\n", start_year, num_steps);
		for (int i = 0; i < num_steps; i++)
			v_s_step_params[i].dump_info();
	}
}s_dcff_params_t;

typedef struct s_comp_info {
	char	name[128];			//��˾��
	double	share_nums;			//�ܹɱ������ڣ�
	double	total_income;		//��Ӫ�գ��ڣ�
	double	net_income;			//�������ڣ�
	double	net_assets;			//���ʲ����ڣ�
	double	liability_ratio;	//��ծ��
	double	ebit;				//Ϣ˰ǰ����
	double	tax_ratio;			//��ҵ����˰��

	void dump_info() {
		printf("��˾��	�ܹɱ������ڣ�	��Ӫ�գ��ڣ�	�������ڣ�	���ʲ�����)	��ծ��	EBIT	˰��\n");
		printf("%s	%.2f	%.2f	%.2f	%.2f	%.3f	%.2f	%.2f\n",
			name, share_nums, total_income, net_income, net_assets, liability_ratio, ebit, tax_ratio);
	}
}s_comp_info_t;

typedef struct s_comp_ev_res
{
	double			enterpise_value;		//��ҵ��ֵ����ֵ��
	double			non_op_assets_value;	//�Ǿ�Ӫ���ʲ���ֵ
	double			debt_value;				//ծ���ֵ����ֵ��
	double			minority_value;			//�����ɶ���ֵ
	double			equity_value;			//Ȩ���ֵ����ֵ��
	double			share_price;			//��Ʊ�۸�
	double			pe;

	void dump_info() {
		printf("��ҵ��ֵ	�Ǿ�Ӫ���ʲ���ֵ	ծ���ֵ	�����ɶ�Ȩ���ֵ	Ȩ���ֵ	ÿ�ɼ۸�Ԫ��	PE\n");
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

	//������׶β�������FCFF
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

	//������׶�����ծ���ֵ
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

	//����������������FCFF
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

	//����������������ծ���ֵ
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

	//�����ֵ���
	comp_ev_res.enterpise_value = std::accumulate(v_dis_fcff.begin(), v_dis_fcff.end(), 0.0f);
	comp_ev_res.debt_value = std::accumulate(v_dis_debt_fcf.begin(), v_dis_debt_fcf.end(), 0.0f);
	comp_ev_res.equity_value = comp_ev_res.enterpise_value + comp_ev_res.non_op_assets_value -
		comp_ev_res.debt_value - comp_ev_res.minority_value;
	comp_ev_res.share_price = comp_ev_res.equity_value / comp_info.share_nums;
	comp_ev_res.pe = comp_ev_res.equity_value / comp_info.net_income;

	//��ӡ���������FCFF
	printf("#########���������FCFF�����ֽ���############\n");
	printf("���	FCFF(��)	���	FCFF(��)\n");
	for (int i = 0; i < v_dis_fcff.size(); i++) {
		if (i % 2 == 0 && i != 0)
			printf("\n%d	%.2f\t", dcff_params.start_year + i, v_dis_fcff[i]);
		else
			printf("%d	%.2f\t", dcff_params.start_year + i, v_dis_fcff[i]);
	}
	double ratio = dis_fcff_forever / comp_ev_res.enterpise_value;
	printf("\n-----------------------------------\n");
	printf("����FCFF������	��ֵ�ٷֱ�\n");
	printf("���������֣�	%.3f\n", 1.0f - ratio);
	printf("��������  ��	%.3f\n", ratio);
	printf("\n");

	return 0;
}