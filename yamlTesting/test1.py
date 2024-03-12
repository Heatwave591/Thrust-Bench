import yaml

with open("D:/Thrust Bench/Thrust-Bench/yamlTesting/pp.yml", "r") as pp:
    data = yaml.load(pp, Loader=yaml.SafeLoader)

print(data)