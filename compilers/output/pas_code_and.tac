		VAR	i
		VAR	j
		VAR	__temp__2
		VAR	__temp__5
		VAR	__temp__8
		VAR	__temp__11
		VAR	__temp__12
		GOTO	example
example:		ASSIGN	0	i
		ASSIGN	6	j
		LT	i	1	__temp__label__3
		ASSIGN	0	__temp__2
		GOTO	__temp__label__4
__temp__label__3:		ASSIGN	1	__temp__2
__temp__label__4:		GT	j	5	__temp__label__6
		ASSIGN	0	__temp__5
		GOTO	__temp__label__7
__temp__label__6:		ASSIGN	1	__temp__5
__temp__label__7:		AND	__temp__2	__temp__5	__temp__8
		NE	__temp__8	0	__block__label__1
		GOTO	__block__label__9

__block__label__1:		ADD	j	1	__temp__11
		ASSIGN	__temp__11	j
		GOTO	__block__label__10

__block__label__9:		SUB	i	1	__temp__12
		ASSIGN	__temp__12	i
		GOTO	__block__label__10

__block__label__10:		APARAM	i
		CALL	writeln
		APARAM	j
		CALL	writeln


