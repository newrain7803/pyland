#import Player from characters/player 

#player_one = Player()

""" Auto generate objects based on level data """
""" Load in saved states """

#print("Creating the level")
#move_north()
#player_one.move_north()

#croc_one.follow_path("north, east, south")

#portal_one.set_destination("level_two")


#boulder_one.move_north()

#boulder_one.callback_test(lambda: boulder_one.callback_test(lambda: print("hohohoho")))

boulder_one.focus()
boulder_one.move_south(lambda: boulder_one.move_south(lambda: boulder_one.move_south(lambda: boulder_one.move_south(lambda: boulder_one.move_south(lambda: boulder_one.move_south())))))
boulder_one.move_south()
boulder_one.move_south()
boulder_one.move_south()
boulder_one.move_south()
boulder_one.move_south()
boulder_one.move_south()
boulder_one.move_south()

player_one.focus()
player_one.move_south()

portal_one.focus()
portal_one.move_west()
portal_one.move_west()
portal_one.move_west()
portal_one.move_west()
portal_one.move_west()
portal_one.move_west()


boulder_four.focus()
boulder_four.move_east()
boulder_four.move_south()

boulder_four.test(game.getDialogue("welcome"))

boulder_four.focus()
#boulder_four.callback_test(lambda: boulder_four.callback_test(lambda: boulder_four.callback_test(lambda: print(dialogue))))
boulder_four.callback_test(lambda: boulder_four.callback_test(lambda: boulder_four.callback_test(lambda: print(game.getDialogue("welcome")))))
print("wohooo")
