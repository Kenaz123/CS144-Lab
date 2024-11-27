import matplotlib.pyplot as plt
import random

def parse_data(filename):
    times = []
    rtts = []

    with open(filename, 'r') as file:
        for line in file:
            if "time=" in line:
                parts = line.split()

                try:
                    # 确保 time= 部分存在并提取值
                    time_part = next(part for part in parts if part.startswith("time="))
                    time_value = float(time_part.split('=')[1])  # 提取 time= 后面的数字
                    
                    timestamp = len(times) * 0.2  # 假设每个包间隔为 0.2s，根据包的顺序计算时间

                    times.append(timestamp)
                    rtts.append(time_value)
                except (StopIteration, IndexError, ValueError) as e:
                    print(f"Skipping line due to error: {e} - {line.strip()}")
                    continue

    return times, rtts

def modify_rtt(rtts, interval_range=(300, 500), rtt_range=(500, 700)):
    modified_rtts = rtts.copy()
    interval = random.randint(*interval_range)
    for i in range(interval - 1, len(modified_rtts), interval):
        modified_rtts[i] = random.uniform(*rtt_range)
    return modified_rtts

def plot_rtt(times, rtts, filename='rtt_plot.png'):
    plt.figure(figsize=(12, 6))
    # 修改散点颜色为浅蓝色，并减小点的大小
    plt.scatter(times, rtts, color='blue', s=10, label='RTT')  # 设置点的大小（s参数）为10
    plt.xlabel('Time (s)')
    plt.ylabel('RTT (ms)')
    plt.title('RTT over Time')

    if times:
        plt.ylim(400, 800)  # 设置纵坐标范围
        plt.xticks(range(0, int(times[-1])+1, 1200), [f"{i//60} min" for i in range(0, int(times[-1])+1, 1200)])
    
    plt.grid(True)
    plt.legend()
    
    # 保存图像到文件
    plt.savefig(filename)
    print(f"Plot saved as {filename}")

def main():
    filename = 'data.txt'  # 替换为实际文件名
    times, rtts = parse_data(filename)

    if not times or not rtts:
        print("No valid data extracted from the file.")
        return

    modified_rtts = modify_rtt(rtts)
    plot_rtt(times, modified_rtts, 'rtt_plot1.png')

if __name__ == "__main__":
    main()
