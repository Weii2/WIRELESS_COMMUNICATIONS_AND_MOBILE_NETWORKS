#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <cmath>
#include <queue>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <tuple>
using namespace std;

class Compare
{
public:
    bool operator() (tuple<double, double, int, int, double> p1, tuple<double, double, int, int, double> p2) {
        return get<4>(p1) > get<4>(p2);
    }
};

void change_direction(tuple<double, double, int, int, double> &t)
{
	int x = rand() % 5;
	if(x==3) {
		get<2>(t)++;
		if(get<2>(t)==4)
			get<2>(t)=0;
	}
	else if(x==4){
		get<2>(t)--;
		if(get<2>(t)==-1)
			get<2>(t)=3;
	}
}

double cal_distance(tuple<double, double, int, int, double> &car, double MS_pos[2])
{
	return pow((get<0>(car)-MS_pos[0]), 2.0) + pow((get<1>(car)-MS_pos[1]), 2.0);
}

double cal_power(tuple<double, double, int, int, double> &car, const int &index, double MS[4][2])
{
	return 100.0 - 33.0 - 10.0 * log10( cal_distance(car, MS[index]) );
}

int which_MS(tuple<double, double, int, int, double> car, double MS[4][2])
{
	vector<double> dis;
	for(int i=0; i<4; ++i)
		dis.push_back(cal_distance(car, MS[i]));
	return min_element(dis.begin(),dis.end()) - dis.begin();
}

double P_best(tuple<double, double, int, int, double> &car, double MS[4][2], int &handoff)
{
	int index = which_MS(car, MS);
	double p = cal_power(car, index, MS);
	if(get<3>(car) != index && p>=10) {
		handoff++;
		get<3>(car) = index;
	}
	return cal_power(car, index, MS);
}

double P_threshold(tuple<double, double, int, int, double> &car, double MS[4][2], int &handoff_sec, double T)
{
	double p = cal_power(car, get<3>(car), MS);
	if(p < T) {
		int index = which_MS(car, MS);
		double p_new =  cal_power(car, index, MS);
		if(get<3>(car) != index && p_new>=10) {
			handoff_sec++;
			get<3>(car) = index;
			return p_new;
		}
	}
	return p;
}

double P_entropy(tuple<double, double, int, int, double> &car, double MS[4][2], int &handoff, double E)
{
	double p = cal_power(car, get<3>(car), MS);
	int index = which_MS(car, MS);
	double p_new = cal_power(car, index, MS);
	if(get<3>(car) != index && p_new >= p+E && p_new >= 10) {
		handoff++;
		get<3>(car) = index;
		return p_new;
	}
	return p;
}

double P_my(tuple<double, double, int, int, double> &car, double MS[4][2], int &handoff, double T, double E)
{
	double p = cal_power(car, get<3>(car), MS);
	if(p < T && p>=10) {
		int index = which_MS(car, MS);
		double p_new = cal_power(car, index, MS);
		if(get<3>(car) != index && p_new >= p+E) {
			handoff++;
			get<3>(car) = index;
			return p_new;
		}
	}
	return p;
}

int main()
{
	srand( time(NULL) );
	
	double MS[4][2]= {{360, 680}, {660, 658}, {330, 350}, {640, 310}};
	const double T=15.0, E=13.0;
	int handoff=0, handoff_sec=0, policy=0;			//change policy		max->0	min->1, T=10
	tuple<double, double, int, int, double> car;	//x, y, direction(0:up, 1:r, 2:down, 3:l), which MS, arrival time
	int time_count=0, total_car=0;
	double power_one_car=0, power_all=0, power_avg=0;

	ofstream outFile("hanoff per sec2.csv");
	
	mt19937 rng(time(NULL));
	double sumArrivalTimes, newArrivalTime;
	queue<double> one_dot_arrival;
	vector<tuple<double, double, int, int, double>> all_arrival, all_car_inSys;
	
	double lambda = 1.0/5.0;						//change lambda
	exponential_distribution<double> arrival(lambda);
	for(int i=0; i<36; ++i) {
		sumArrivalTimes=0;
		while(sumArrivalTimes <= 86400) {
			newArrivalTime = arrival.operator()(rng);
			sumArrivalTimes = sumArrivalTimes+newArrivalTime;
			if(i <= 8) {
				get<0>(car) = 100*(i+1);	get<1>(car) = 0;	get<2>(car) = 0;	get<3>(car) = which_MS(car, MS);	get<4>(car) = sumArrivalTimes;
			}
			else if(i>8 && i<=17) {
				get<0>(car) = 0;	get<1>(car) = 100*(i-8);	get<2>(car) = 1;	get<3>(car) = which_MS(car, MS);	get<4>(car) = sumArrivalTimes;
			}
			else if(i>17 && i<=26) {
				get<0>(car) = 100*(i-17);	get<1>(car) = 1000;	get<2>(car) = 2;	get<3>(car) = which_MS(car, MS);	get<4>(car) = sumArrivalTimes;
			}
			else if(i>26 && i<=35) {
				get<0>(car) = 1000;	get<1>(car) = 100*(i-26);	get<2>(car) = 3;	get<3>(car) = which_MS(car, MS);	get<4>(car) = sumArrivalTimes;
			}
			all_arrival.push_back(car);
		}
	}
	sort(all_arrival.begin(), all_arrival.end(), Compare());
	for(int time=0; time<86400; time++) {	//0.001
		handoff_sec=0;
		while(!all_arrival.empty() && get<4>(all_arrival[all_arrival.size()-1]) <= time) {
			all_car_inSys.push_back(all_arrival[all_arrival.size()-1]);
			all_arrival.pop_back();
		}
		for(int i=0; i<all_car_inSys.size(); ++i) {
			if(get<2>(all_car_inSys[i]) == 0)
				get<1>(all_car_inSys[i]) += 10;	//0.01
			else if(get<2>(all_car_inSys[i]) == 1)
				get<0>(all_car_inSys[i]) += 10;
			else if(get<2>(all_car_inSys[i]) == 2)
				get<1>(all_car_inSys[i]) -= 10;
			else if(get<2>(all_car_inSys[i]) == 3)
				get<0>(all_car_inSys[i]) -= 10;
			if(int(get<0>(all_car_inSys[i])) <= 0 || int(get<0>(all_car_inSys[i])) >= 1000 || int(get<1>(all_car_inSys[i])) <= 0 || int(get<1>(all_car_inSys[i])) >= 1000) {
				all_car_inSys.erase(all_car_inSys.begin() + i);
				continue;
			}
			if(int(get<0>(all_car_inSys[i])) % 100 == 0 && int(get<1>(all_car_inSys[i])) % 100 == 0)
				change_direction(all_car_inSys[i]);
			if(policy==0) {
				power_one_car = P_best(all_car_inSys[i], MS, handoff_sec);
				if(power_one_car >= 10)
					power_all +=  power_one_car;
			}
			else if(policy==1) {
				power_one_car = P_threshold(all_car_inSys[i], MS, handoff_sec, T);
				if(power_one_car >= 10)
					power_all +=  power_one_car;
			}
			else if(policy==2) {
				power_one_car = P_entropy(all_car_inSys[i], MS, handoff_sec, E);
				if(power_one_car >= 10)
					power_all +=  power_one_car;
			}
			else if(policy==3) {
				power_one_car = P_my(all_car_inSys[i], MS, handoff_sec, T, E);
				if(power_one_car >= 10)
					power_all +=  power_one_car;
			}
		}
		total_car += all_car_inSys.size();
		handoff += handoff_sec;
		outFile << handoff_sec << endl;
	}
	power_avg = power_all/total_car;
	cout << "N_all = " << total_car << endl;
	cout << "handoff = " << handoff << endl;
	cout << "AVG Power = " << power_avg << endl;
	return 0;
}

