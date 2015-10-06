#include <cstring>

#include <cpu_monitor.h>
#include <debug_utils.h>

cpu_monitor::cpu_monitor(const char* title, const char* x_title, const char* y_title, unsigned int size)
{
    gp = popen("gnuplot -persist -noraise", "w");
    if(!gp)
    {
        printb(d_cpu_monitor, "error opening gnuplot");
    }
    fprintf(gp, "\n");

    x.resize(size);
    y.resize(size);

    for(int i = 0; i < size; i++)
    {
        x[i] = 0;
        y[i] = 0;
    }

    //little hack to shut up gnuplot
    x[1] = 1;
    y[1] = 1;
    y[2] = 0;
    y[3] = 1;

    strcpy(this->title, title);
    strcpy(this->x_title, x_title);
    strcpy(this->y_title, y_title);

    char filename[128];
    strcpy(filename, title);
    strcat(filename, ".csv");
    fout = fopen(filename, "w");
}

cpu_monitor::~cpu_monitor()
{
    fclose(gp);
    fclose(fout);
}

void cpu_monitor::add(uint64_t x, uint32_t y)
{
    this->x.erase(this->x.begin());
    this->x.push_back(x);
    this->y.erase(this->y.begin());
    this->y.push_back(y);

    fprintf(fout, "%d, %d\n", x, y);
}

void cpu_monitor::plot()
{
    fprintf(gp, "set xlabel \"%s\"\n", x_title);
    fprintf(gp, "set ylabel \"%s\"\n", y_title);
    fprintf(gp, "set terminal qt\n");
    fprintf(gp, "plot [%llu:%u][-5:110] '-' title '%s' with lines\n", x.front(), x.back(), title);
    //fprintf(gp, "plot [%llu:%u][-5:110] '-' title '%s' with linespoints\n", x.front(), x.back(), title);

    for(int i = 0; i < x.size() - 1; i++)
    {
        fprintf(gp, "%llu %d\n", x[i], y[i]);
    }

    fprintf(gp, "e\n");
    fflush(gp);
}

