#pragma once

#include <math.h>
#include <vector>
#include <numeric>
#include <functional>

typedef struct s_step_params {
	int	years;					//����
	double	grow_rate;			//������������
	double	ke;					//��Ȩ�ʱ��ɱ�
	double	pre_y_netincome;	//ǰһ��ľ�����
	double	roe;				//��ǰ�׶�roe

	void dump_info() {
		printf("����	������������	ke	������(PY)	ROE(STEP)\n");
		printf("%d	%.2f	%.4f	%.2f	%.4f\n", years, grow_rate, ke, pre_y_netincome, roe);
	}
}s_step_params_t;

#define	NUM_STEPS 8
typedef struct s_dcf_params {
	int		start_year;		//��ʼ���
	int		num_steps;		//�׶���
	s_step_params_t	v_s_step_params[NUM_STEPS];	//�׶β���

	void dump_info() {
		printf("��ʼ���	�׶���\n");
		printf("%d	%d\n", start_year, num_steps);
		for (int i = 0; i < num_steps; i++)
			v_s_step_params[i].dump_info();
	}
}s_dcfe_params_t;

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
	double			equity_value;		//Ȩ���ֵ����ֵ��
	double			share_price_y;		//��Ʊ�۸񣨹��������ڣ����������ڣ�
	double			share_price_t;		//��Ʊ�۸񣨺������ڣ�
	double			pe_y;
	double			pe_t;

	void dump_info() {
		printf("Ȩ���ֵ���ڣ�	ÿ�ɼ۸�Y��Ԫ��	ÿ�ɼ۸�T��Ԫ��	PE_Y	PE_T\n");
		printf("%.2f	%.2f	%.2f	%.2f	%.2f\n", equity_value, share_price_y, share_price_t, pe_y, pe_t);
	}
}s_comp_ev_res_t;

int dcfe(s_comp_info_t comp_info, s_dcfe_params_t dcf_params, s_comp_ev_res_t& comp_ev_res, double safe_coef = 0.8) {
	std::vector<double>		v_net_income;
	std::vector<double>		v_fcfe;
	std::vector<double>		v_dis_fcfe;
	v_fcfe.reserve(16);
	v_dis_fcfe.reserve(16);
	
	//������׶β�������FCFE
	for (int i = 0; i < dcf_params.num_steps - 1; i++) {
		if (i > 0) {
			dcf_params.v_s_step_params[i].pre_y_netincome = v_net_income.back();
		}
		else {
			dcf_params.v_s_step_params[i].pre_y_netincome = comp_info.net_income;
		}

		for (int j = 0; j < dcf_params.v_s_step_params[i].years; j++) {			
			double net_income = dcf_params.v_s_step_params[i].pre_y_netincome * pow(dcf_params.v_s_step_params[i].grow_rate + 1, j + 1.0);
			v_net_income.emplace_back(net_income); 
			double fcfe = net_income * ( 1 - dcf_params.v_s_step_params[i].grow_rate / dcf_params.v_s_step_params[i].roe);
			v_fcfe.emplace_back(fcfe);
			double ke = pow(dcf_params.v_s_step_params[i].ke + 1, j + 1.0);
			for (int m = i - 1; m >= 0; m--) {
				ke *= pow(dcf_params.v_s_step_params[m].ke + 1, dcf_params.v_s_step_params[m].years);
			}
			double dis_fcfe = fcfe / ke;
			v_dis_fcfe.emplace_back(dis_fcfe);
		}
	}

	//����������������FCFE
	double g_f = dcf_params.v_s_step_params[dcf_params.num_steps - 1].grow_rate;
	double r_f = dcf_params.v_s_step_params[dcf_params.num_steps - 1].ke;
	double roe_f  = dcf_params.v_s_step_params[dcf_params.num_steps - 1].roe;
	double net_income_forever1{ 0.0f };
	double fcfe_forever1{ 0.0f };
	double fcfe_forever{ 0.0f };
	if (!v_net_income.empty()) {
		net_income_forever1 = v_net_income.back() * (1 + g_f);
		fcfe_forever1 = net_income_forever1 * (1 - g_f / roe_f);
		fcfe_forever = fcfe_forever1 / (r_f - g_f);
	}
	else {
		net_income_forever1 = comp_info.net_income * (1 + g_f);
		fcfe_forever1 = net_income_forever1 * (1 - g_f / roe_f);
		fcfe_forever = fcfe_forever1 / (r_f - g_f);
	}
	v_net_income.emplace_back(net_income_forever1);
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

	//��ӡ��������ֹ�Ȩ�ʱ������ֽ���
	printf("#########���������FCFE�����ֽ���############\n");
	printf("���	FCFE(��)	���	FCFE(��)\n");
	for (int i = 0; i < v_dis_fcfe.size(); i++) {
		if(i % 2 == 0 && i != 0)
			printf("\n%d	%.2f\t", dcf_params.start_year + i, v_dis_fcfe[i]);
		else
			printf("%d	%.2f\t", dcf_params.start_year + i, v_dis_fcfe[i]);
	}
	double ratio = dis_fcfe_forever / comp_ev_res.equity_value;
	printf("\n-----------------------------------\n");
	printf("����FCFE������	��ֵ�ٷֱ�\n");
	printf("���������֣�	%.3f\n", 1.0f - ratio);
	printf("��������  ��	%.3f\n", ratio);
	printf("\n");

	return 0;
}