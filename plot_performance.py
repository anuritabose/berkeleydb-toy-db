import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

sns.set(style="whitegrid")
os.makedirs("perf_plots", exist_ok=True)

# Plot: WAL Size vs Load Size
df_wal = pd.read_csv("perf_logs/wal_size.csv")
plt.figure()
sns.lineplot(data=df_wal, x="load_size", y="wal_size_bytes", marker="o")
plt.title("BerkeleyDB: WAL Size vs Load Size")
plt.xlabel("Number of Operations")
plt.ylabel("WAL Size (bytes)")
plt.savefig("perf_plots/bdb_wal_size.png")

# Plot: Recovery Latency vs Load Size
df_recovery = pd.read_csv("perf_logs/recovery_latency.csv")
plt.figure()
sns.lineplot(data=df_recovery, x="load_size", y="recovery_time_ms", marker="o", color="green")
plt.title("BerkeleyDB: Recovery Latency vs Load Size")
plt.xlabel("Number of Operations")
plt.ylabel("Recovery Time (ms)")
plt.savefig("perf_plots/bdb_recovery_latency.png")

# Plot: Transaction Throughput
df_tp = pd.read_csv("perf_logs/throughput.csv")
plt.figure()
sns.lineplot(data=df_tp, x="load_size", y="ops_per_sec", hue="workload_type", marker="o")
plt.title("BerkeleyDB: Throughput by Workload Type")
plt.xlabel("Number of Operations")
plt.ylabel("Transactions per Second")
plt.legend(title="Workload Type")
plt.savefig("perf_plots/bdb_throughput.png")

# Plot: Lock Contention Latency
df_lock = pd.read_csv("perf_logs/lock_contention.csv")
plt.figure()
sns.lineplot(data=df_lock, x="num_conflicts", y="time_ms", marker="o", color="red")
plt.title("BerkeleyDB: Lock Contention Latency")
plt.xlabel("Number of Conflicting Transactions")
plt.ylabel("Latency (ms)")
plt.savefig("perf_plots/bdb_lock_contention.png")

# Plot: MVCC Version Chain Scaling
df_mvcc = pd.read_csv("perf_logs/mvcc_scaling.csv")
plt.figure()
sns.lineplot(data=df_mvcc, x="num_versions", y="txn_time_ms", marker="o", color="purple")
plt.title("BerkeleyDB: MVCC Write Latency vs Version Count")
plt.xlabel("Number of Versions Written to One Key")
plt.ylabel("Transaction Time (ms)")
plt.savefig("perf_plots/bdb_mvcc_scaling.png")

# Plot: Undo Recovery Cost
df_undo = pd.read_csv("perf_logs/undo_cost.csv")
plt.figure()
sns.lineplot(data=df_undo, x="num_uncommitted", y="recovery_time_ms", marker="o", color="brown")
plt.title("BerkeleyDB: Undo Cost vs Number of Uncommitted Ops")
plt.xlabel("Number of Uncommitted Updates")
plt.ylabel("Recovery Time (ms)")
plt.savefig("perf_plots/bdb_undo_cost.png")

print("BerkeleyDB performance plots saved to perf_plots/")
