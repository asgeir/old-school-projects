		VAR	x
		VAR	y
		VAR	__temp__11
		VAR	__temp__13
		VAR	__temp__18
		GOTO	test
test:		FPARAM	x
		VAR	t
		VAR	i
		VAR	__temp__2
		VAR	__temp__3
		VAR	__temp__6
		VAR	__temp__7
		GOTO	__block__label__1
__block__label__1:		ASSIGN	0	i
		MULT	2	x	__temp__2
		UMINUS	__temp__2	__temp__3
		ASSIGN	__temp__3	t
		GOTO	__block__label__4

__block__label__4:		LT	i	4	__temp__label__8
		ASSIGN	0	__temp__7
		GOTO	__temp__label__9
__temp__label__8:		ASSIGN	1	__temp__7
__temp__label__9:		NE	__temp__7	0	__block__label__5
		GOTO	__block__label__10

__block__label__5:		ADD	i	1	__temp__6
		ASSIGN	__temp__6	i
		GOTO	__block__label__4

__block__label__10:		ASSIGN	t	test
		RETURN

test:		MULT	2	3.14	__temp__11
		ASSIGN	__temp__11	y
		GT	x	2	__temp__label__14
		ASSIGN	0	__temp__13
		GOTO	__temp__label__15
__temp__label__14:		ASSIGN	1	__temp__13
__temp__label__15:		NE	__temp__13	0	__block__label__12
		GOTO	__block__label__16

__block__label__12:		ASSIGN	0.12345	y
		GOTO	__block__label__17

__block__label__16:		APARAM	3.3
		CALL	test
		ASSIGN	test	y
		ADD	x	2	__temp__18
		ASSIGN	__temp__18	x
		GOTO	__block__label__17

__block__label__17:

