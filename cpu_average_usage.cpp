#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <unistd.h>
#include <iomanip>
#include <fstream>
using namespace std;

#define PROCSTAT "/proc/stat"

struct CPUSTat{
    char name[20];
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long allbasytime;
    unsigned long long allruntime;
};

// 根据proc/stat推算cpu个数
int get_number_of_cores(){
    FILE* fp = fopen(PROCSTAT, "r");
    assert(fp != NULL);
    char buffer[100];
    int cpu_number = 0;
    while(!feof(fp)){
        // 从文件中读取一行
        fgets(buffer, sizeof(buffer), fp);
        if(buffer[0] == 'c' && buffer[1] == 'p' &&buffer[2] == 'u'){
            cpu_number++;
        }
    }
    fclose(fp);
    return cpu_number-1;
}

vector<CPUSTat> old_stat; // n * 12
vector<CPUSTat> new_stat;

vector<CPUSTat> read_cpu(){
    FILE* fp = fopen(PROCSTAT, "r");
    assert(fp != NULL);
    int cores = get_number_of_cores();
    vector<CPUSTat> stats;
    for(int i=0; i<cores+1; i++){
        CPUSTat stat;
        fscanf(fp, "%s%llu%llu%llu%llu%llu%llu%llu%llu%llu", stat.name, &stat.user, &stat.nice, &stat.system,
            &stat.idle, &stat.iowait, &stat.irq, &stat.softirq, &stat.steal, &stat.guest); 
        stat.allbasytime = stat.user + stat.nice + stat.system + stat.irq + stat.softirq ;
        stat.allruntime = stat.allbasytime + stat.idle + stat.iowait + stat.steal + stat.guest;
        stats.push_back(stat); 
    }
    fclose(fp);
    return stats;
}

vector<double> get_cpus_percentage(vector<CPUSTat>& stat1, vector<CPUSTat>& stat2){
    int cores = get_number_of_cores();
    vector<double> percentage;
    for(int i=0; i<cores; i++){
        double cpu_ave_rate = 1.0 * (double)(stat2[i].allbasytime - stat1[i].allbasytime) / (double)(stat2[i].allruntime - stat1[i].allruntime) * 100.0;
        percentage.push_back(cpu_ave_rate);
    }
    return percentage;
}

int main() {
    int cores = get_number_of_cores();
    cout << cores << std::endl;
    std::cout << "CPU";
    for(int i=0; i<cores; i++){
        std::cout << "\tCPU-" <<i;
    }
    std::cout << std::endl;
    cout << "-----------------------------------------------------------------------" << endl;
    ofstream fout;
    fout.open("cpu.log");
    while(1){
        old_stat = read_cpu();
        sleep(1);
        new_stat = read_cpu();
        vector<double> percentage = get_cpus_percentage(old_stat, new_stat);
        double start_point = 5.0;
        // 当比5.0大的时候就开始记录
        if(percentage[0] > start_point){
            for(int i=0; i<percentage.size(); i++){
                printf("%.2lf\t", percentage[i]);
                if(i == 0){
                    fout << percentage[i] << endl;
                }
            }
            cout << endl;
            cout << "-----------------------------------------------------------------------" << endl;
            if(percentage[0] < start_point){break;}
        }else{
            for(int i=0; i<percentage.size(); i++){
                printf("%.2lf\t", percentage[i]);
            }
            cout << endl;
            cout << "-----------------------------------------------------------------------" << endl;
            
        }
    }
    fout.close();
    return 0;
}


 