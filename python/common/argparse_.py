import argparse

parse = argparse.ArgumentParser()
parse.add_argument("--save", type=bool, default=False)

parse.add_argument("--asave", action='store_false')
parse.add_argument("--bsave", action='store_true')
args = parse.parse_args()

print(args.save) # True for whatever input
print(args.asave)
print(args.bsave)
