import sys
import os
import subprocess
import time
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

def norm(v):
  if (len(v)==1): return v
  miv = np.min(v)
  mav = np.max(v)
  if (mav-miv == 0): return v
  return (v-miv)/(mav-miv)

def to_png(x, y, t, f):
  plt.legend()
  plt.xlabel(x); plt.ylabel(y); plt.title(t)
  plt.savefig(f); plt.clf()

def main():
  if (len(sys.argv) != 1):
    print("Usage: <./program name>")
    return 1

  img_dir = "img"
  out_dir = "output"
  imgs = os.listdir(img_dir)
  imgs = [img for img in imgs if img != ".DS_Store"]

  os.chdir("../")
  path = "./extra-credit/"
  fn = "time.txt"

  f = open(f"{path}{fn}", "w")
  f.write(f"real,user,syst,cpu,mem\n")
  f.close()

  MAX_NUM_THREADS = 100
  for T in range(1, MAX_NUM_THREADS+1):
    print(f"-- #{T} --")

    fmt = f"\"%e,%U,%S,%P,%M\""
    cmd = f"\\time -f {fmt} -o {path}{fn} -a"
    subprocess.Popen(f"{cmd} ./image_rotation {path}{img_dir} {path}{out_dir} {T} 180", shell=True).wait()

    print()

  os.chdir(path)
  df = pd.read_csv(fn)

  thds = np.arange(1, len(df)+1)
  real = df['real'].to_numpy()
  user = df['user'].to_numpy()
  syst = df['syst'].to_numpy()

  cpu = [(int)(pt.split('%')[0]) for pt in df['cpu'] if '%' in pt]
  mem = df['mem'].to_numpy()

  plt.plot(thds, norm(real), '+:m', label='real')
  to_png("# of threads", "time", "Program Analysis", "time.png")

  plt.plot(thds, (cpu), '+:b', label='cpu')
  to_png("# of threads", "utilization", "Program Analysis", "cpu_util.png")

  plt.plot(thds, (mem), '+:g', label='mem')
  to_png("# of threads", "utilization", "Program Analysis", "mem_util.png")

  plt.close()

if __name__=="__main__":
  main()
