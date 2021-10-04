#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <cmath>
#include <queue>
#include <vector>
#include <algorithm>
#include <iomanip>
using namespace std;

int main()
{
	int num_of_server[3]={1, 5, 10}, queue_size;
	int num_of_arr=0, num_of_de=0, clear=0;
	int servering_num=0, count=0, bp_count=0;
	int total_arr=0, total_de=0, total_blocked=0;
	double bpro=0.0;
	vector<double> blocking_pro(72), blocking_pro_queue(72);
	float lambda, mu;
	int interval;
	queue<double> arr, dep;
	
	vector<double> server_depart_time, queue_time;
	double newArrivalTime=0, sumArrivalTimes=0, newDepartTime=0;

 	mt19937 rng(time(NULL));
 	int c=0;
 	
 	for(int i=0; i<72; ++i) {
 		blocking_pro[i]=0;	blocking_pro_queue[i]=0;
	 }

	for(int runs=0; runs<10; ++runs) {
		bp_count=0;	count=0;
		for(int i=0; i<3; ++i) {
			for(lambda=0.01; lambda<=10; lambda=lambda*10) {
				exponential_distribution<double> arrival(lambda);
				for(mu=0.01; mu<=10.24; mu=mu*4) {
					exponential_distribution<double> departure(mu);
					total_arr=0;	total_de=0;	 total_blocked=0;
					servering_num=0;
					sumArrivalTimes=0;
					
					server_depart_time.clear();
	    			server_depart_time.shrink_to_fit();	
					
					while(sumArrivalTimes <= 100000) {
						newArrivalTime = arrival.operator()(rng);
						sumArrivalTimes = sumArrivalTimes+newArrivalTime;
						arr.push(sumArrivalTimes);
						total_arr++;
					}
					
					for(int j=0; j<num_of_server[count]; ++j) {
						newDepartTime = departure.operator()(rng);
						server_depart_time.push_back(newDepartTime+arr.front());
						arr.pop();
					}
					while(!arr.empty()) {
						newDepartTime = departure.operator()(rng);
						sort(server_depart_time.begin(), server_depart_time.end());
						if(arr.front() >= server_depart_time[0])
							server_depart_time[0] = newDepartTime+arr.front();
						else
							total_blocked++;
						arr.pop();
					}
					bpro = (double)total_blocked / (double)total_arr;
					blocking_pro[bp_count] = blocking_pro[bp_count]+bpro;
					bp_count++;
				}
			}
			count++;
		}
	}
	for(int i=0; i<72; ++i)
		blocking_pro[i] = blocking_pro[i]/10.0;
	
	vector<double> server_depart_time_for_queue;
	count=0;
	for(int runs=0; runs<10; ++runs) {
		bp_count=0;	count=0;
		for(int i=0; i<3; ++i) {
			queue_size=num_of_server[count];
			for(lambda=0.01; lambda<=10; lambda=lambda*10) {
				exponential_distribution<double> arrival(lambda);
				for(mu=0.01; mu<=10.24; mu=mu*4) {
					exponential_distribution<double> departure(mu);
					total_arr=0;	total_de=0;	 total_blocked=0;
					servering_num=0;
					sumArrivalTimes=0;
					
					server_depart_time_for_queue.clear();
	    			server_depart_time_for_queue.shrink_to_fit();			
					queue_time.clear();
	    			queue_time.shrink_to_fit();
	    			
					while(sumArrivalTimes <= 100000) {
						newArrivalTime = arrival.operator()(rng);
						sumArrivalTimes = sumArrivalTimes+newArrivalTime;
						arr.push(sumArrivalTimes);
						total_arr++;
					}
					
					for(int j=0; j<num_of_server[count]; ++j) {
						newDepartTime = departure.operator()(rng);
						server_depart_time_for_queue.push_back(newDepartTime+arr.front());
						arr.pop();
					}
					for(int j=0; j<num_of_server[count]; ++j) {
						queue_time.push_back(arr.front());
						arr.pop();
					}
					while(!arr.empty()) {
						newDepartTime = departure.operator()(rng);
						sort(server_depart_time_for_queue.begin(), server_depart_time_for_queue.end());
						if(queue_time[0] >= server_depart_time_for_queue[0]) {
							server_depart_time_for_queue[0] = queue_time[0]+newDepartTime;
						
							queue_time.erase(queue_time.begin());
							queue_time.push_back(arr.front());
							arr.pop();
						}		
						else {
							double interval=server_depart_time_for_queue[0]-queue_time[0];
							while(arr.front()<server_depart_time_for_queue[0]) {
								total_blocked++;
								if(!arr.empty())
									arr.pop();
								else
									break;
							}
							queue_time[0] = server_depart_time_for_queue[0];
						}
					}
					
					bpro = (double)total_blocked / (double)total_arr;
					blocking_pro_queue[bp_count] = blocking_pro_queue[bp_count]+bpro;
					bp_count++;
				}
			}
			count++;
		}
	}
	for(int i=0; i<72; ++i)
		blocking_pro_queue[i]=blocking_pro_queue[i]/10;
	
	ofstream outFile1("Blocking_Probability_Table_Q=0.csv");
	ofstream outFile2("Blocking_Probability_Table_Q=S.csv");
	outFile1 << "S," << "Blocking Probability and Erlang value For Q=0" <<endl;
	outFile2 << "S," << "Blocking Probability and Erlang value For Q=S" <<endl;
	count=0;
	for(int i=0; i<3; ++i) {
		outFile1 << num_of_server[i] << ",";
		outFile2 << num_of_server[i] << ",";
		for(lambda=0.01; lambda<=10; lambda=lambda*10) {
			for(mu=0.01; mu<=10.24; mu=mu*4) {
				outFile1 << blocking_pro[count];
				outFile2 << blocking_pro_queue[count];
				if(++count%24!=0) {
					outFile1 << ",";
					outFile2 << ",";
				}
			}
		}
		outFile1 << endl << ",";
		outFile2 << endl << ",";
		count = count-24;
		for(lambda=0.01; lambda<=10; lambda=lambda*10) {
			for(mu=0.01; mu<=10.24; mu=mu*4) {
				outFile1 << lambda/mu;
				outFile2 << lambda/mu;
				if(++count%24!=0) {
					outFile1 << ",";
					outFile2 << ",";
				}
			}
		}
		outFile1 << endl << endl;
		outFile2 << endl << endl;
	}
	
	return 0;
}

