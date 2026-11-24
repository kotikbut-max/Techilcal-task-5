#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int read_int(const char *prompt, int min_v, int max_v) {
    int val;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &val) == 1 && getchar() == '\n' && val >= min_v && val <= max_v) {
            return val;
        }
        clear_input();
        printf("Ошибка: введите целое число от %d до %d.\n", min_v, max_v);
    }
}

double read_double(const char *prompt) {
    double val;
    while (1) {
        printf("%s", prompt);
        if (scanf("%lf", &val) == 1 && getchar() == '\n') {
            return val;
        }
        clear_input();
        printf("Ошибка: введите корректное число.\n");
    }
}

void read_str(const char *prompt, char *buf, size_t size) {
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, (int)size, stdin)) {
            buf[strcspn(buf, "\r\n")] = '\0';
            if (strlen(buf) > 0) return;
        }
        printf("Ошибка: строка не должна быть пустой.\n");
    }
}

void print_char_repr(FILE *out, unsigned char ch) {
    if (ch == '\n') fprintf(out, "\"\\n\"");
    else if (ch == '\t') fprintf(out, "\"\\t\"");
    else if (ch == '\r') fprintf(out, "\"\\r\"");
    else if (isprint(ch)) fprintf(out, "\"%c\"", ch);
    else fprintf(out, "\"0x%02X\"", ch);
}

/* Общая функция построения двух диаграмм (используется в заданиях 2 и 3) */
void draw_histograms(const char *chars, const unsigned int *counts, size_t n) {
    if (n == 0) {
        printf("Нет данных для построения диаграмм.\n");
        return;
    }
    unsigned int max_v = 0;
    unsigned long long sum = 0;
    for (size_t i = 0; i < n; i++) {
        if (counts[i] > max_v) max_v = counts[i];
        sum += counts[i];
    }
    if (max_v == 0 || sum == 0) return;

    printf("\nДиаграмма (а): относительно максимума (5%% = *)\n");
    for (size_t i = 0; i < n; i++) {
        double pct = (double)counts[i] / max_v * 100.0;
        int stars = (int)(pct / 5.0 + 1e-9);
        print_char_repr(stdout, (unsigned char)chars[i]);
        printf(" (%5.1f%%): ", pct);
        for (int s = 0; s < stars; s++) putchar('*');
        putchar('\n');
    }

    printf("\nДиаграмма (б): относительно суммы (2%% = *)\n");
    for (size_t i = 0; i < n; i++) {
        double pct = (double)counts[i] / sum * 100.0;
        int stars = (int)(pct / 2.0 + 1e-9);
        print_char_repr(stdout, (unsigned char)chars[i]);
        printf(" (%5.1f%%): ", pct);
        for (int s = 0; s < stars; s++) putchar('*');
        putchar('\n');
    }
}

/* ================= ЗАДАНИЕ 1: Метод Гаусса ================= */
void task1_gauss(void) {
    int n = read_int("Введите количество неизвестных (1..50): ", 1, 50);

    double **a = malloc(n * sizeof(double *));
    double *x = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) a[i] = malloc((n + 1) * sizeof(double));

    for (int i = 0; i < n; i++) {
        printf("Уравнение %d:\n", i + 1);
        for (int j = 0; j < n; j++) {
            char msg[32];
            snprintf(msg, sizeof(msg), "  a[%d][%d] = ", i + 1, j + 1);
            a[i][j] = read_double(msg);
        }
        char msg_b[32];
        snprintf(msg_b, sizeof(msg_b), "  b[%d]    = ", i + 1);
        a[i][n] = read_double(msg_b);
    }

    int singular = 0;
    for (int col = 0; col < n; col++) {
        int pivot = col;
        for (int r = col + 1; r < n; r++)
            if (fabs(a[r][col]) > fabs(a[pivot][col])) pivot = r;

        if (fabs(a[pivot][col]) < 1e-12) {
            singular = 1;
            break;
        }
        double *tmp = a[col]; a[col] = a[pivot]; a[pivot] = tmp;

        for (int r = col + 1; r < n; r++) {
            double k = a[r][col] / a[col][col];
            for (int j = col; j <= n; j++) a[r][j] -= k * a[col][j];
        }
    }

    if (singular) {
        printf("Определитель матрицы равен нулю. Решение найти невозможно.\n");
    } else {
        for (int i = n - 1; i >= 0; i--) {
            double s = a[i][n];
            for (int j = i + 1; j < n; j++) s -= a[i][j] * x[j];
            x[i] = s / a[i][i];
        }
        printf("Решение:\n");
        for (int i = 0; i < n; i++) printf("  x[%d] = %.6f\n", i + 1, x[i]);
    }

    for (int i = 0; i < n; i++) free(a[i]);
    free(a);
    free(x);
}

/* ================= ЗАДАНИЕ 2: Двоичный файл ================= */
void task2_binary(void) {
    char fname[256];
    read_str("Введите имя двоичного файла: ", fname, sizeof(fname));
    FILE *fp = fopen(fname, "rb");
    if (!fp) {
        printf("Ошибка: не удалось открыть файл.\n");
        return;
    }

    char chars[1024];
    unsigned int counts[1024];
    size_t n = 0;

    while (n < 1024 && fread(&chars[n], sizeof(char), 1, fp) == 1) {
        if (fread(&counts[n], sizeof(unsigned int), 1, fp) != 1) break;
        n++;
    }
    fclose(fp);
    draw_histograms(chars, counts, n);
}

/* ================= ЗАДАНИЕ 3: Частота символов в тексте ================= */
void task3_frequency(void) {
    char in_name[256], out_name[256];
    read_str("Введите имя входного текстового файла: ", in_name, sizeof(in_name));
    read_str("Введите имя выходного файла для записи процентов: ", out_name, sizeof(out_name));

    FILE *in = fopen(in_name, "r");
    if (!in) {
        printf("Ошибка: не удалось открыть входной файл.\n");
        return;
    }

    unsigned int freq[256] = {0};
    unsigned long long total = 0;
    int c;
    while ((c = fgetc(in)) != EOF) {
        freq[(unsigned char)c]++;
        total++;
    }
    fclose(in);

    if (total == 0) {
        printf("Файл пуст.\n");
        return;
    }

    char chars[256];
    unsigned int counts[256];
    size_t n = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            chars[n] = (char)i;
            counts[n] = freq[i];
            n++;
        }
    }

    draw_histograms(chars, counts, n);

    FILE *out = fopen(out_name, "w");
    if (!out) {
        printf("Ошибка: не удалось создать выходной файл.\n");
        return;
    }
    for (size_t i = 0; i < n; i++) {
        print_char_repr(out, (unsigned char)chars[i]);
        fprintf(out, " – %.1f%%\n", (double)counts[i] / total * 100.0);
    }
    fclose(out);
    printf("Процентное содержание записано в '%s'.\n", out_name);
}

/* ================= ЗАДАНИЕ 4: Разбиение строк по 80 символов ================= */
void print_justified(FILE *out, char words[][81], int cnt, int words_len) {
    if (cnt == 1) {
        fprintf(out, "%s\n", words[0]);
        return;
    }
    int spaces = 80 - words_len;
    int gaps = cnt - 1;
    int base = spaces / gaps;
    int extra = spaces % gaps;

    for (int i = 0; i < cnt; i++) {
        fprintf(out, "%s", words[i]);
        if (i < gaps) {
            int sp = base + (i < extra ? 1 : 0);
            for (int s = 0; s < sp; s++) fputc(' ', out);
        }
    }
    fputc('\n', out);
}

void format_line_80(FILE *out, char *line) {
    size_t pos = 0;
    while (line[pos] && isspace((unsigned char)line[pos])) pos++;
    size_t len = strlen(line + pos);
    while (len > 0 && (line[pos + len - 1] == '\n' || line[pos + len - 1] == '\r')) len--;
    line[pos + len] = '\0';

    if (len < 80) {
        fprintf(out, "%s\n", line + pos);
        return;
    }

    char words[512][81];
    int wcnt = 0;
    char *tok = strtok(line + pos, " \t\r\n");
    while (tok && wcnt < 512) {
        strncpy(words[wcnt], tok, 80);
        words[wcnt][80] = '\0';
        wcnt++;
        tok = strtok(NULL, " \t\r\n");
    }

    int i = 0;
    while (i < wcnt) {
        int cur_len = (int)strlen(words[i]);
        int cnt = 1;
        while (i + cnt < wcnt && cur_len + cnt + (int)strlen(words[i + cnt]) <= 80) {
            cur_len += (int)strlen(words[i + cnt]);
            cnt++;
        }
        if (i + cnt < wcnt) {
            print_justified(out, &words[i], cnt, cur_len);
        } else {
            for (int k = 0; k < cnt; k++) {
                if (k > 0) fputc(' ', out);
                fprintf(out, "%s", words[i + k]);
            }
            fputc('\n', out);
        }
        i += cnt;
    }
}

void task4_wrap80(void) {
    char in_name[256], out_name[256];
    read_str("Введите имя исходного файла: ", in_name, sizeof(in_name));
    read_str("Введите имя нового файла: ", out_name, sizeof(out_name));

    FILE *in = fopen(in_name, "r");
    FILE *out = fopen(out_name, "w");
    if (!in || !out) {
        printf("Ошибка открытия файлов.\n");
        if (in) fclose(in);
        if (out) fclose(out);
        return;
    }

    char line[4096];
    while (fgets(line, sizeof(line), in)) {
        format_line_80(out, line);
    }

    fclose(in);
    fclose(out);
    printf("Готово! Результат сохранён в '%s'.\n", out_name);
}

void run_task(int num) {
    if (num == 1) task1_gauss();
    else if (num == 2) task2_binary();
    else if (num == 3) task3_frequency();
    else if (num == 4) task4_wrap80();
    else printf("Ошибка: такого задания нет.\n");
}

int main(int argc, char *argv[]) {
    printf("Студент: Бутаков Григорий, группа ИТПМ-126\n");
    printf("Лабораторная работа № 5\n");

    if (argc > 1) {
        if (argc == 3 && strcmp(argv[1], "-n") == 0 && strlen(argv[2]) == 1 && argv[2][0] >= '1' && argv[2][0] <= '4') {
            run_task(argv[2][0] - '0');
            return 0;
        }
        printf("Использование: %s [-n <1..4>]\n", argv[0]);
        return 1;
    }

    while (1) {
        printf("\n1. Метод Гаусса\n2. Двоичный файл (диаграммы)\n3. Частота символов в тексте\n4. Форматирование строк по 80 символов\n0. Выход\n");
        int choice = read_int("Выберите пункт: ", 0, 4);
        if (choice == 0) break;
        run_task(choice);
    }
    return 0;
}
