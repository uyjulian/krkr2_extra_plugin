#ifndef FOREACH_MAX
#define FOREACH_MAX 20
#endif

#if defined(FOREACH) && defined(FOREACH_START) && defined(FOREACH_END)

#undef  FOREACH_COMMA

#undef  FOREACH_COUNT
#define FOREACH_COUNT 0
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#define FOREACH_COMMA
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT)
FOREACH
#endif

#undef  FOREACH_COMMA
#define FOREACH_COMMA ,

#undef  FOREACH_COUNT
#define FOREACH_COUNT 1
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 2
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 3
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 4
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 5
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 6
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 7
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 8
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 9
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 10
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 11
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 12
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11) EXT(12)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11),EXT(12)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 13
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11) EXT(12) EXT(13)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11),EXT(12),EXT(13)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 14
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11) EXT(12) EXT(13) EXT(14)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11),EXT(12),EXT(13),EXT(14)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 15
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11) EXT(12) EXT(13) EXT(14) EXT(15)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11),EXT(12),EXT(13),EXT(14),EXT(15)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 16
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11) EXT(12) EXT(13) EXT(14) EXT(15) EXT(16)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11),EXT(12),EXT(13),EXT(14),EXT(15),EXT(16)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 17
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11) EXT(12) EXT(13) EXT(14) EXT(15) EXT(16) EXT(17)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11),EXT(12),EXT(13),EXT(14),EXT(15),EXT(16),EXT(17)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 18
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11) EXT(12) EXT(13) EXT(14) EXT(15) EXT(16) EXT(17) EXT(18)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11),EXT(12),EXT(13),EXT(14),EXT(15),EXT(16),EXT(17),EXT(18)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 19
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11) EXT(12) EXT(13) EXT(14) EXT(15) EXT(16) EXT(17) EXT(18) EXT(19)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11),EXT(12),EXT(13),EXT(14),EXT(15),EXT(16),EXT(17),EXT(18),EXT(19)
FOREACH
#endif

#undef  FOREACH_COUNT
#define FOREACH_COUNT 20
#if    (FOREACH_START <= FOREACH_COUNT && FOREACH_COUNT <= FOREACH_END)
#undef  FOREACH_SPACE_EXT
#define FOREACH_SPACE_EXT(EXT) EXT(1) EXT(2) EXT(3) EXT(4) EXT(5) EXT(6) EXT(7) EXT(8) EXT(9) EXT(10) EXT(11) EXT(12) EXT(13) EXT(14) EXT(15) EXT(16) EXT(17) EXT(18) EXT(19) EXT(20)
#undef  FOREACH_COMMA_EXT
#define FOREACH_COMMA_EXT(EXT) EXT(1),EXT(2),EXT(3),EXT(4),EXT(5),EXT(6),EXT(7),EXT(8),EXT(9),EXT(10),EXT(11),EXT(12),EXT(13),EXT(14),EXT(15),EXT(16),EXT(17),EXT(18),EXT(19),EXT(20)
FOREACH
#endif

#undef  FOREACH_COUNT
#undef  FOREACH_COMMA
#undef  FOREACH_SPACE_EXT
#undef  FOREACH_COMMA_EXT
#endif
