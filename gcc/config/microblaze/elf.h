/* Definitions of target machine for GNU compiler for Xilinx MicroBlaze.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   Contributed by Michael Eager <eager@eagercon.com>.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

/* Overrides from standard elfos.h */

#undef  MAX_OFILE_ALIGNMENT
#define MAX_OFILE_ALIGNMENT		(32768*8)

#undef  TARGET_ASM_OUTPUT_IDENT
#define TARGET_ASM_OUTPUT_IDENT microblaze_asm_output_ident

#undef  SKIP_ASM_OP
#define SKIP_ASM_OP			"\t.space\t"
#undef  STRING_ASM_OP
#define STRING_ASM_OP			"\t.asciz\t"
#undef  READONLY_DATA_SECTION_ASM_OP
#define READONLY_DATA_SECTION_ASM_OP	"\t.rodata"
#undef  INIT_SECTION_ASM_OP
#define INIT_SECTION_ASM_OP     	"\t.section\t.init,\"ax\""
#undef  FINI_SECTION_ASM_OP
#define FINI_SECTION_ASM_OP     	"\t.section\t.fini,\"ax\""

#undef  ASM_GENERATE_INTERNAL_LABEL
#define ASM_GENERATE_INTERNAL_LABEL(LABEL,PREFIX,NUM)			\
  sprintf ((LABEL), "*%s%s%ld", (LOCAL_LABEL_PREFIX), (PREFIX), (long)(NUM))

/* ASM_OUTPUT_ALIGNED_COMMON and ASM_OUTPUT_ALIGNED_LOCAL

   Unfortunately, we still need to set the section explicitly. Somehow,
   our binutils assign .comm and .lcomm variables to the "current" section
   in the assembly file, rather than where they implicitly belong. We need to
   remove this explicit setting in GCC when binutils can understand sections
   better.  */
#undef  ASM_OUTPUT_ALIGNED_COMMON
#define ASM_OUTPUT_ALIGNED_COMMON(FILE, NAME, SIZE, ALIGN)		\
  do									\
    {									\
      if ((SIZE) > 0 && (SIZE) <= INT_MAX				\
	  && (int) (SIZE) <= microblaze_section_threshold		\
	  && TARGET_XLGPOPT)						\
	{								\
	  switch_to_section (sbss_section);				\
	}								\
      else								\
	{								\
	  switch_to_section (bss_section);				\
	}								\
      fprintf (FILE, "%s", COMMON_ASM_OP);				\
      assemble_name ((FILE), (NAME));					\
      fprintf ((FILE), "," HOST_WIDE_INT_PRINT_UNSIGNED",%u\n",		\
	       (SIZE), (ALIGN) / BITS_PER_UNIT);			\
      ASM_OUTPUT_TYPE_DIRECTIVE (FILE, NAME, "object");			\
    }									\
  while (0)

#undef  ASM_OUTPUT_ALIGNED_LOCAL
#define ASM_OUTPUT_ALIGNED_LOCAL(FILE, NAME, SIZE, ALIGN)		\
  do									\
    {									\
      if ((SIZE) > 0 && (SIZE) <= INT_MAX				\
	  && (int) (SIZE) <= microblaze_section_threshold		\
	  && TARGET_XLGPOPT)						\
	{								\
	  switch_to_section (sbss_section);				\
	}								\
      else								\
	{								\
	  switch_to_section (bss_section);				\
	}								\
      fprintf (FILE, "%s", LCOMMON_ASM_OP);				\
      assemble_name ((FILE), (NAME));					\
      fprintf ((FILE), "," HOST_WIDE_INT_PRINT_UNSIGNED",%u\n",		\
	       (SIZE), (ALIGN) / BITS_PER_UNIT);			\
      ASM_OUTPUT_TYPE_DIRECTIVE (FILE, NAME, "object");			\
    }									\
  while (0)

#undef  ASM_WEAKEN_LABEL
#define ASM_WEAKEN_LABEL(FILE,NAME)	\
  do					\
    {					\
      fputs ("\t.weakext\t", FILE);	\
      assemble_name (FILE, NAME);	\
      fputc ('\n', FILE);		\
    }					\
  while (0)

/* Write the extra assembler code needed to declare an object properly.  */
#undef  ASM_DECLARE_OBJECT_NAME
#define ASM_DECLARE_OBJECT_NAME(FILE, NAME, DECL)			\
  do									\
    {									\
      fprintf (FILE, "%s", TYPE_ASM_OP);			        \
      assemble_name (FILE, NAME);					\
      putc (',', FILE);							\
      fprintf (FILE, TYPE_OPERAND_FMT, "object");			\
      putc ('\n', FILE);						\
      size_directive_output = 0;					\
      if (!flag_inhibit_size_directive					\
	  && DECL_SIZE (DECL))						\
	{								\
	  size_directive_output = 1;					\
	  fprintf (FILE, "%s", SIZE_ASM_OP);				\
	  assemble_name (FILE, NAME);					\
	  fprintf (FILE, "," HOST_WIDE_INT_PRINT_DEC "\n",		\
	  int_size_in_bytes (TREE_TYPE (DECL)));			\
	}								\
      microblaze_declare_object (FILE, NAME, "", ":\n", 0);		\
    }									\
  while (0)

#undef  ASM_FINISH_DECLARE_OBJECT
#define ASM_FINISH_DECLARE_OBJECT(FILE, DECL, TOP_LEVEL, AT_END)	\
  do									\
    {									\
      const char *name = XSTR (XEXP (DECL_RTL (DECL), 0), 0);		\
      if (!flag_inhibit_size_directive					\
	  && DECL_SIZE (DECL)						\
	  && ! AT_END && TOP_LEVEL					\
	  && DECL_INITIAL (DECL) == error_mark_node			\
	  && !size_directive_output)					\
	{								\
	  size_directive_output = 1;					\
	  fprintf (FILE, "%s", SIZE_ASM_OP);			        \
	  assemble_name (FILE, name);					\
	  fprintf (FILE, "," HOST_WIDE_INT_PRINT_DEC "\n",		\
		   int_size_in_bytes (TREE_TYPE (DECL)));		\
	}								\
    }									\
  while (0)

/* Added for declaring size at the end of the function.  */
#undef  ASM_DECLARE_FUNCTION_SIZE
#define ASM_DECLARE_FUNCTION_SIZE(FILE, FNAME, DECL)			\
  do									\
    {									\
      if (!flag_inhibit_size_directive)					\
	{								\
	  char label[256];						\
	  static int labelno;						\
	  labelno++;							\
	  ASM_GENERATE_INTERNAL_LABEL (label, "Lfe", labelno);		\
	  (*targetm.asm_out.internal_label) (FILE, "Lfe", labelno);	\
	  fprintf (FILE, "%s", SIZE_ASM_OP);				\
	  assemble_name (FILE, (FNAME));				\
	  fprintf (FILE, ",");						\
	  assemble_name (FILE, label);					\
	  fprintf (FILE, "-");						\
	  assemble_name (FILE, (FNAME));				\
	  putc ('\n', FILE);						\
	}								\
    }									\
  while (0)
