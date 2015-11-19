		VAR	j
		VAR	n
		VAR	__temp__12
		VAR	__temp__13
		GOTO	example
fact:		FPARAM	n
		VAR	k
		VAR	__temp__3
		VAR	__temp__8
		VAR	__temp__9
		GOTO	__block__label__1
__block__label__1:		LT	n	1	__temp__label__4
		ASSIGN	0	__temp__3
		GOTO	__temp__label__5
__temp__label__4:		ASSIGN	1	__temp__3
__temp__label__5:		NE	__temp__3	0	__block__label__2
		GOTO	__block__label__6

__block__label__2:		ASSIGN	1	fact
		GOTO	__block__label__7

__block__label__6:		SUB	n	1	__temp__8
		APARAM	__temp__8
		CALL	fact
		ASSIGN	fact	k
		MULT	n	k	__temp__9
		ASSIGN	__temp__9	fact
		GOTO	__block__label__7

__block__label__7:		RETURN

example:		ASSIGN	5	j
		GOTO	__block__label__10

__block__label__10:		GT	j	0	__temp__label__14
		ASSIGN	0	__temp__13
		GOTO	__temp__label__15
__temp__label__14:		ASSIGN	1	__temp__13
__temp__label__15:		NE	__temp__13	0	__block__label__11
		GOTO	__block__label__16

__block__label__11:		APARAM	j
		CALL	fact
		ASSIGN	fact	n
		APARAM	n
		CALL	writeln
		SUB	j	1	__temp__12
		ASSIGN	__temp__12	j
		GOTO	__block__label__10

__block__label__16:

