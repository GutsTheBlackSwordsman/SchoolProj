#include "s21_grep.h"

int main(int argc, char **argv) {
  Flags flag = {0};
  int flag_printable_name = 0;
  parser(argc, argv, &flag);
  while (optind < argc) {
    if (argc - optind > 1) flag_printable_name = 1;
    output(argv, &flag, &flag_printable_name);
    optind++;
  }

  return 0;
}

void parser(int argc, char **argv, Flags *flag) {
  int opt = 0;
  while ((opt = getopt_long(argc, argv, "e:civlnhsf:o", NULL, 0)) != -1) {
    switch (opt) {
      case 'e':
        flag->e = 1;
        strcat(flag->comp_string, optarg);
        strcat(flag->comp_string, "|");
        break;
      case 'c':
        flag->c = 1;
        break;
      case 'i':
        flag->i = 1;
        break;
      case 'v':
        flag->v = 1;
        break;
      case 'l':
        flag->l = 1;
        break;
      case 'n':
        flag->n = 1;
        break;
      case 'h':
        flag->h = 1;
        break;
      case 's':
        flag->s = 1;
        break;
      case 'o':
        if (flag->v != 1) {
          flag->o = 1;
        }
        break;
      case '?':
        fprintf(stderr,
                "usage: grep [-abcdDEFGHhIiJLlMmnOopqRSsUVvwXxZz] [-A num] [-B "
                "num] [-C[num]] [-e pattern] [-f file] [--binary-files=value] "
                "[--color=when] [--context[=num]] [--directories=action] "
                "[--label] [--line-buffered] [--null] [pattern] [file ...]");
        break;
    }
  }
  if (!flag->e) {
    if (argc > optind) {
      strcat(flag->comp_string, argv[optind]);
    }
    optind++;
  }
  if (flag->e) {
    flag->comp_string[strlen(flag->comp_string) - 1] = '\0';
  }
}

void output(char **argv, Flags *flag, int *flag_printable_name) {
  FILE *file;
  regmatch_t start;
  int count_lines = 0;
  int match_counter = 0;

  file = fopen(argv[optind], "r");
  if (file == NULL) {
    if (!flag->s)
      fprintf(stderr, "grep: %s: No such file or directory\n", argv[optind]);
    return;
  } else {
    regex_t reg;
    int valve_flag = REG_EXTENDED;
    if (flag->i) valve_flag |= REG_ICASE;

    regcomp(&reg, flag->comp_string, valve_flag);
    while ((fgets(flag->line_from_file, BUFFER, file)) != NULL) {
      count_lines++;
      int compare = regexec(&reg, flag->line_from_file, 1, &start, 0);
      if (!compare) match_counter++;

      if (flag->v) compare = !compare;
      if (compare == 0 && flag->n) printf("%d:", count_lines);
      if (*flag_printable_name == 1 && compare == 0 && !flag->h && !flag->c)
        printf("%s:", argv[optind]);
      if ((compare == 0 && !flag->l && !flag->c && !flag->o))
        printf("%s", flag->line_from_file);
      if (flag->o && !flag->c && !flag->l && compare == 0)
        print_match_only(flag, &start, &reg);
    }
    if (flag->c && flag->l) {
      if (match_counter > 1) match_counter = 1;
      printf("%s:%d\n", argv[optind], match_counter);
    }
    if (flag->c) printf("%d\n", match_counter);
    if (flag->l && match_counter != 0) printf("%s\n", argv[optind]);
    regfree(&reg);
  }
  fclose(file);
}

void print_match_only(Flags *flag, regmatch_t *start, regex_t *reg) {
  char *only_string = flag->comp_string;
  while (!regexec(reg, only_string, 1, start, 0)) {
    printf("%.*s\n", (int)(start->rm_eo - start->rm_so),
           only_string + start->rm_so);
    only_string += start->rm_eo;
  }
}
