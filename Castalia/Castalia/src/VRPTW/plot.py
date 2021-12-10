import os
import matplotlib.pyplot as plt

with open('MinDistance.txt') as f:
    for line in f:
        pass
    last_line = line

min_lengths = [float	(s) for s in last_line.split('\t') if s]

with open('AvgDistance.txt') as f:
    for line in f:
        pass
    last_line = line

avg_lengths = [float(s) for s in last_line.split('\t') if s]

# plt.plot(min_lengths)
# plt.show()

print(min_lengths[-1])

fig, (ax1, ax2) = plt.subplots(1, 2)
fig.suptitle('Horizontally stacked subplots')
ax1.plot(min_lengths)
ax2.plot(avg_lengths)
ax1.set_title('Min length')
ax2.set_title('Avg length')
plt.show()