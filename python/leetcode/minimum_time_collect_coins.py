from itertools import product
from dataclasses import dataclass
from enum import Enum
from tkinter.tix import Tree
from typing import Optional, List, Iterable, TypeVar
import itertools
import math

class Side(Enum):
	LEFT = 1
	RIGHT = 2

def CalculateTookTime(side: Side, coinBlock: List[int], NLeft: int, player: int) -> int:
	if side == Side.RIGHT:
		rightSideCoins = coinBlock[:NLeft]
		if rightSideCoins:
			return rightSideCoins[-1] - player
		return -1
	elif side == Side.LEFT:
		leftSideCoins = coinBlock[NLeft:]
		if leftSideCoins:
			return player - leftSideCoins[0]
		return -1

def min_time_to_collect_all_coins(players, coins):
	sortedPlayers = sorted(players)
	sortedCoins = sorted(coins)
	lenPlayers = len(sortedPlayers)
	lenCoins = len(sortedCoins)
	if not lenCoins:
		return 0
	if not lenPlayers:
		return -1

	# Extract coin blocks, e.g. | coin1 coin2 | Player1 | coin3 coin4 |
	coinBlocks = []
	coinBlock = []
	playerIndex = 0
	involvedPlayers = []
	headChoices: Optional[int] = None
	tailChoices: Optional[int] = None
	for coin in sortedCoins:
		while playerIndex < lenPlayers and coin > players[playerIndex]:
			if coinBlock:
				coinBlocks.append(coinBlock)
				coinBlock = []
				if not involvedPlayers:
					if playerIndex:
						involvedPlayers.append(sortedPlayers[playerIndex-1])
					else:
						headChoices = 0
						involvedPlayers.append(None)
				involvedPlayers.append(sortedPlayers[playerIndex])
			playerIndex += 1
		coinBlock.append(coin)
	coinBlocks.append(coinBlock)
	if playerIndex < lenPlayers:
		involvedPlayers.append(sortedPlayers[playerIndex])
	else:
		tailChoices = len(coinBlock)
		involvedPlayers.append(None)

	print(coinBlocks)
	print(involvedPlayers)

	# Travel all permutation
	# select the first n coin in coinBlock for the player on the left side of coinBlock
	numCoinForLeftPlayer = [list(range(len(coinBlock)+1)) for coinBlock in coinBlocks]
	numCoinForLeftPlayer[0] = [headChoices] if headChoices is not None else numCoinForLeftPlayer[0]
	numCoinForLeftPlayer[-1] = [tailChoices] if tailChoices is not None else numCoinForLeftPlayer[-1]
	permutations = list(itertools.product(*numCoinForLeftPlayer))
	print(permutations)

	# Assign player and calculate the best result
	# Player/None | coin | coin | Player | coin | Player/None
	bestPermutation = permutations[0]
	shortestTime = -1
	for permutation in permutations:
		tookTime = -1
		# Player1 | coin1 | coin2 | ...
		if involvedPlayers[0] is not None:
			tookTime = CalculateTookTime(Side.RIGHT, coinBlocks[0], permutation[0], involvedPlayers[0])
		# ... | coin1 | coin2 | Player1
		if involvedPlayers[-1] is not None:
			tookTime = max(tookTime, CalculateTookTime(Side.LEFT, coinBlocks[-1], permutation[-1], involvedPlayers[-1]))
		for index, player in enumerate(involvedPlayers[1:-1]):
			leftTookTime = CalculateTookTime(Side.LEFT, coinBlocks[index], permutation[index], player)
			rightTookTime = CalculateTookTime(Side.RIGHT, coinBlocks[index+1], permutation[index+1], player)
			tempTookTime = max(leftTookTime, rightTookTime)
			if leftTookTime > 0 and rightTookTime > 0:
				tempTookTime += min(leftTookTime, rightTookTime)*2
			tookTime = max(tookTime, tempTookTime)
		print(f"permutation: {permutation}, took time: {tookTime}")
		if tookTime < 0:
			continue
		elif shortestTime < 0 or tookTime < shortestTime:
			shortestTime = tookTime
			bestPermutation = permutation

	print(f"Result: permutation: {bestPermutation}, took time: {shortestTime}")

players = [100,200,300]
coins = [5, 201,199,151,234, 354]

# 计算最短时间
result = min_time_to_collect_all_coins(players, coins)