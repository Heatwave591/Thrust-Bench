import yaml

with open("D:/Thrust Bench/Thrust-Bench/yamlTesting/Ramp_Normal.yaml", "r") as pp:
    data = yaml.safe_load(pp)
    print(data)