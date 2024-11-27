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

def plot_correlation(rtts, filename='correlation.png'):
    plt.figure(figsize=(12, 6))
    
    # 构建 (RTT_n, RTT_{n+1}) 数据对
    x_values = rtts[:-1]
    y_values = rtts[1:]

    # 绘制散点图
    plt.scatter(x_values, y_values, color='blue', s=10, label='RTT correlation')
    
    # 设置标题和标签
    plt.xlabel('RTT of n (ms)')
    plt.ylabel('RTT of n+1 (ms)')
    plt.title('RTT correlation over time')
    
    # 设置横纵坐标范围和刻度
    plt.xlim(440, 520)
    plt.ylim(440, 520)
    plt.xticks(range(440, 521, 20))  # 每 20 ms 刻度
    plt.yticks(range(440, 521, 20))  # 每 20 ms 刻度

    plt.grid(True)
    plt.legend()
    
    # 保存图像到文件
    plt.savefig(filename)
    print(f"Correlation plot saved as {filename}")

def main():
    filename = 'data.txt'  # 替换为实际文件名
    times, rtts = parse_data(filename)

    if not times or not rtts:
        print("No valid data extracted from the file.")
        return

    modified_rtts = modify_rtt(rtts)
    plot_correlation(modified_rtts, 'correlation.png')

if __name__ == "__main__":
    main()
