#pragma once

#include <math.h>
#include <vector>
#include <numeric>
#include <functional>

typedef struct s_step_params {
	int	years;				//����
	double	grow_rate;		//fcfe������
	double	ke;
	double	pre_y_fcfe;		//ǰһ���fcfe

	void dump_info() {
		printf("����	FCFE������	ke	ǰһ���FCFE\n");
		printf("%d	%.2f	%.4f	%.2f\n", years, grow_rate, ke, pre_y_fcfe);
	}
}s_step_params_t;

#define	NUM_STEPS 8
typedef struct s_dcfe_params {
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
	double	fcfe;			//��Ȩ�ʱ������ֽ������ڣ�

	void dump_info() {
		printf("��˾��	�ܹɱ������ڣ�	��Ӫ�գ��ڣ�	�������ڣ�	���ʲ����ڣ�	FCFE���ڣ�\n");
		printf("%s	%.2f	%.2f	%.2f	%.2f	%.2f\n", 
			name, share_nums, total_income, net_income, net_assets, fcfe);
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

int dcdv(s_comp_info_t comp_info, s_dcfe_params_t dcf_params, s_comp_ev_res_t& comp_ev_res, double safe_coef = 0.8) {
	std::vector<double>		v_fcfe;
	std::vector<double>		v_dis_fcfe;
	v_fcfe.reserve(16);
	v_dis_fcfe.reserve(16);
	
	//������׶β�������FCFE
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

	//����������������FCFE
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

	//��ӡ���������FCFE
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