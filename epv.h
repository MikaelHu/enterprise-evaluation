#pragma once

#include <math.h>
#include <vector>
#include <numeric>
#include <functional>

typedef struct s_step_params {
	int	years;				//����
	double	grow_rate;		//Ȩ��������
	double	ke;				//Ȩ��ɱ�
	//double	required_ret;	//������
	double	pre_y_eq;		//ǰһ���Ȩ�澻ֵ
	double	roe;			//Ȩ��ر���

	void dump_info() {
		printf("����	g	ke	ǰһ���Ȩ�澻ֵ	roe\n");
		printf("%d	%.2f 	%.4f	%.2f	%.2f\n", years, grow_rate, ke, pre_y_eq, roe);
	}
}s_step_params_t;

#define	NUM_STEPS 8
typedef struct s_epv_params {
	int		start_year;		//��ʼ���
	int		num_steps;		//�׶���
	s_step_params_t	v_s_step_params[NUM_STEPS];	//�׶β���

	void dump_info() {
		printf("��ʼ���	�׶���\n");
		printf("%d	%d\n", start_year, num_steps);
		for (int i = 0; i < num_steps; i++)
			v_s_step_params[i].dump_info();
	}
}s_epv_params_t;

typedef struct s_comp_info {
	char	name[128];		//��˾��
	double	share_nums;		//�ܹɱ������ڣ�
	double	total_income;	//��Ӫ�գ��ڣ�
	double	net_income;		//�������ڣ�
	double	net_assets;		//���ʲ����ڣ�

	void dump_info() {
		printf("��˾��	�ܹɱ������ڣ�	��Ӫ�գ��ڣ�	�������ڣ�	���ʲ����ڣ�\n");
		printf("%s	%.2f	%.2f	%.2f	%.2f\n",
			name, share_nums, total_income, net_income, net_assets);
	}
}s_comp_info_t;

typedef struct s_comp_ev_res
{
	double			enterpise_value;	//Ȩ���ֵ����ֵ��
	double			share_price_y;		//��Ʊ�۸񣨹��������ڣ����������ڣ�
	double			share_price_t;		//��Ʊ�۸񣨺������ڣ�
	double			pe_y;
	double			pe_t;

	void dump_info() {
		printf("Ȩ���ֵ���ڣ�	ÿ�ɼ۸�Y��Ԫ��	ÿ�ɼ۸�T��Ԫ��	PE_Y	PE_T\n");
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

	//������׶β������ֹ�Ȩ����
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

	//���������������ֹ�Ȩ����
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

	//��ӡ��������ֹ�Ȩ����
	printf("#########��������ֹ�Ȩ����############\n");
	printf("���	EP(��)	���	EP(��)\n");
	for (int i = 0; i < v_dis_ep.size(); i++) {
		if (i % 2 == 0 && i != 0)
			printf("\n%d	%.2f\t", epv_params.start_year + i, v_dis_ep[i]);
		else
			printf("%d	%.2f\t", epv_params.start_year + i, v_dis_ep[i]);
	}
	double ratio = dis_ep_forever / (comp_ev_res.enterpise_value - comp_info.net_assets);
	printf("\n-----------------------------------\n");
	printf("���ֹ�Ȩ���������	��ֵ�ٷֱ�\n");
	printf("���������֣�	%.3f\n", 1.0f - ratio);
	printf("��������  ��	%.3f\n", ratio);
	printf("\n");

	return 0;
}