#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GASTOS 7
#define MAX_ATIVOS 6
#define TAM_LINHA 120

typedef struct {
    char nome[80];
    int idade;
    double rendaMensal;
    double gastos[MAX_GASTOS];
    double totalGastos;
    double sobraInvestir;
    int preenchido;
} Orcamento;

typedef struct {
    char nomes[MAX_ATIVOS][80];
    double precos[MAX_ATIVOS];
    double quantidades[MAX_ATIVOS];
    double valoresAtuais[MAX_ATIVOS];
    double pesos[MAX_ATIVOS];
    int preenchida;
} Carteira;

void limparTela(void);
void pausar(void);
int menuPrincipal(void);
int lerInteiroFaixa(const char *mensagem, int minimo, int maximo);
double lerDoubleMinimo(const char *mensagem, double minimo, int permiteIgual);
void lerTextoObrigatorio(const char *mensagem, char *destino, size_t tamanho);
void inicializarCarteira(Carteira *carteira);
void cadastrarOrcamento(Orcamento *orcamento, const char categorias[MAX_GASTOS][30]);
void escolherPesos(Carteira *carteira);
void definirPerfilAutomatico(Carteira *carteira, int perfil);
void definirPesosManuais(Carteira *carteira);
double somarPesos(const Carteira *carteira);
void cadastrarCarteiraAtual(Carteira *carteira);
void exibirResumo(const Orcamento *orcamento, const Carteira *carteira, const char categorias[MAX_GASTOS][30]);
void simularRebalanceamento(const Orcamento *orcamento, const Carteira *carteira);
void exibirAjudaInvestimentos(void);
double valorAbsoluto(double valor);

int main(void) {
    const char categorias[MAX_GASTOS][30] = {
        "Moradia",
        "Alimentacao",
        "Transporte",
        "Saude",
        "Educacao",
        "Lazer",
        "Outros"
    };

    Orcamento orcamento = {0};
    Carteira carteira;
    int opcao;

    inicializarCarteira(&carteira);

    do {
        opcao = menuPrincipal();

        switch (opcao) {
            case 1:
                cadastrarOrcamento(&orcamento, categorias);
                break;
            case 2:
                escolherPesos(&carteira);
                break;
            case 3:
                cadastrarCarteiraAtual(&carteira);
                break;
            case 4:
                exibirResumo(&orcamento, &carteira, categorias);
                break;
            case 5:
                exibirAjudaInvestimentos();
                break;
            case 0:
                limparTela();
                printf("Obrigado por usar o simulador. Bons estudos e bons investimentos!\n");
                break;
            default:
                printf("Entrada invalida.\n");
                pausar();
                break;
        }
    } while (opcao != 0);

    return 0;
}

void limparTela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar(void) {
    char linha[TAM_LINHA];

    printf("\nPressione ENTER para continuar...");
    fgets(linha, sizeof(linha), stdin);
}

int menuPrincipal(void) {
    limparTela();
    printf("============================================================\n");
    printf(" SIMULADOR DE CARTEIRA DE INVESTIMENTOS E REBALANCEAMENTO\n");
    printf("============================================================\n");
    printf("1 - Informar ganho mensal e gastos\n");
    printf("2 - Definir pesos percentuais da carteira\n");
    printf("3 - Informar valores que ja tenho investidos\n");
    printf("4 - Ver resumo, sobra para investir e rebalanceamento\n");
    printf("5 - Ajuda para leigos: tipos de investimento\n");
    printf("0 - Sair\n");
    printf("============================================================\n");

    return lerInteiroFaixa("Escolha uma opcao: ", 0, 5);
}

int lerInteiroFaixa(const char *mensagem, int minimo, int maximo) {
    char linha[TAM_LINHA];
    char *fim;
    long valor;

    while (1) {
        printf("%s", mensagem);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("Entrada invalida. Tente novamente.\n");
            clearerr(stdin);
            continue;
        }

        errno = 0;
        valor = strtol(linha, &fim, 10);

        while (isspace((unsigned char)*fim)) {
            fim++;
        }

        if (fim == linha || *fim != '\0' || errno == ERANGE) {
            printf("Entrada invalida. Digite apenas numeros inteiros.\n");
            continue;
        }

        if (valor < minimo || valor > maximo) {
            printf("Entrada invalida. Digite um valor entre %d e %d.\n", minimo, maximo);
            continue;
        }

        return (int)valor;
    }
}

double lerDoubleMinimo(const char *mensagem, double minimo, int permiteIgual) {
    char linha[TAM_LINHA];
    char *fim;
    double valor;
    int valorValido;

    while (1) {
        printf("%s", mensagem);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("Entrada invalida. Tente novamente.\n");
            clearerr(stdin);
            continue;
        }

        errno = 0;
        valor = strtod(linha, &fim);

        while (isspace((unsigned char)*fim)) {
            fim++;
        }

        valorValido = permiteIgual ? (valor >= minimo) : (valor > minimo);

        if (fim == linha || *fim != '\0' || errno == ERANGE || !valorValido) {
            if (permiteIgual) {
                printf("Entrada invalida. Digite um numero maior ou igual a %.2f.\n", minimo);
            } else {
                printf("Entrada invalida. Digite um numero maior que %.2f.\n", minimo);
            }
            continue;
        }

        return valor;
    }
}

void lerTextoObrigatorio(const char *mensagem, char *destino, size_t tamanho) {
    char linha[TAM_LINHA];
    char *inicio;
    char *fim;

    while (1) {
        printf("%s", mensagem);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("Entrada invalida. Tente novamente.\n");
            clearerr(stdin);
            continue;
        }

        linha[strcspn(linha, "\n")] = '\0';
        inicio = linha;

        while (isspace((unsigned char)*inicio)) {
            inicio++;
        }

        fim = inicio + strlen(inicio);
        while (fim > inicio && isspace((unsigned char)*(fim - 1))) {
            fim--;
        }
        *fim = '\0';

        if (strlen(inicio) == 0) {
            printf("Entrada invalida. O nome nao pode ficar vazio.\n");
            continue;
        }

        if (tamanho > 0) {
            strncpy(destino, inicio, tamanho - 1);
            destino[tamanho - 1] = '\0';
        }

        return;
    }
}

void inicializarCarteira(Carteira *carteira) {
    const char nomesPadrao[MAX_ATIVOS][80] = {
        "Reserva de emergencia (Tesouro Selic/CDB liquidez diaria)",
        "Renda fixa conservadora (CDB/LCI/LCA/Tesouro Selic)",
        "Tesouro IPCA+ (protege contra inflacao)",
        "Fundos imobiliarios - FIIs (renda mensal)",
        "ETFs de acoes (diversificacao na bolsa)",
        "Caixa para objetivos curtos e oportunidades"
    };
    int i;

    for (i = 0; i < MAX_ATIVOS; i++) {
        strcpy(carteira->nomes[i], nomesPadrao[i]);
        carteira->precos[i] = 1.0;
        carteira->quantidades[i] = 0.0;
        carteira->valoresAtuais[i] = 0.0;
        carteira->pesos[i] = 0.0;
    }

    carteira->preenchida = 0;
}

void cadastrarOrcamento(Orcamento *orcamento, const char categorias[MAX_GASTOS][30]) {
    int i;

    limparTela();
    printf("================ ORCAMENTO MENSAL ================\n");
    printf("Informe seu nome, idade, renda e gastos. Use ponto para centavos.\n");
    printf("Exemplo: 2500.50\n\n");

    lerTextoObrigatorio("Nome do usuario: ", orcamento->nome, sizeof(orcamento->nome));
    orcamento->idade = lerInteiroFaixa("Idade do usuario: ", 1, 120);
    orcamento->rendaMensal = lerDoubleMinimo("Ganho mensal total: R$ ", 0.0, 0);
    orcamento->totalGastos = 0.0;

    for (i = 0; i < MAX_GASTOS; i++) {
        char mensagem[100];
        snprintf(mensagem, sizeof(mensagem), "Gasto mensal com %s: R$ ", categorias[i]);
        orcamento->gastos[i] = lerDoubleMinimo(mensagem, 0.0, 1);
        orcamento->totalGastos += orcamento->gastos[i];
    }

    orcamento->sobraInvestir = orcamento->rendaMensal - orcamento->totalGastos;
    orcamento->preenchido = 1;

    printf("\nUsuario: %s, %d anos\n", orcamento->nome, orcamento->idade);
    printf("\nTotal de gastos: R$ %.2f\n", orcamento->totalGastos);

    if (orcamento->sobraInvestir > 0.0) {
        printf("Sobra mensal para investir: R$ %.2f\n", orcamento->sobraInvestir);
    } else if (orcamento->sobraInvestir == 0.0) {
        printf("Voce nao teve sobra este mes. Antes de investir, tente criar margem no orcamento.\n");
    } else {
        printf("Alerta: seus gastos passaram da renda em R$ %.2f.\n", valorAbsoluto(orcamento->sobraInvestir));
        printf("Prioridade: reduzir despesas ou aumentar renda antes de investir.\n");
    }

    pausar();
}

void escolherPesos(Carteira *carteira) {
    int opcao;

    limparTela();
    printf("=============== PESOS DA CARTEIRA ===============\n");
    printf("Os pesos indicam quanto do dinheiro deve ir para cada investimento.\n");
    printf("A soma obrigatoriamente precisa dar 100%%.\n\n");
    printf("1 - Perfil conservador (menos risco)\n");
    printf("2 - Perfil moderado (equilibrado)\n");
    printf("3 - Perfil arrojado (mais oscilacao)\n");
    printf("4 - Digitar percentuais manualmente\n");
    printf("0 - Voltar\n\n");

    opcao = lerInteiroFaixa("Escolha uma opcao: ", 0, 4);

    if (opcao == 0) {
        return;
    }

    if (opcao >= 1 && opcao <= 3) {
        definirPerfilAutomatico(carteira, opcao);
    } else {
        definirPesosManuais(carteira);
    }

    carteira->preenchida = 1;

    printf("\nPesos definidos com sucesso:\n");
    for (opcao = 0; opcao < MAX_ATIVOS; opcao++) {
        printf("- %-58s %.2f%%\n", carteira->nomes[opcao], carteira->pesos[opcao]);
    }

    pausar();
}

void definirPerfilAutomatico(Carteira *carteira, int perfil) {
    double conservador[MAX_ATIVOS] = {35, 35, 15, 5, 5, 5};
    double moderado[MAX_ATIVOS] = {25, 25, 20, 10, 15, 5};
    double arrojado[MAX_ATIVOS] = {20, 15, 20, 15, 25, 5};
    int i;

    for (i = 0; i < MAX_ATIVOS; i++) {
        if (perfil == 1) {
            carteira->pesos[i] = conservador[i];
        } else if (perfil == 2) {
            carteira->pesos[i] = moderado[i];
        } else {
            carteira->pesos[i] = arrojado[i];
        }
    }
}

void definirPesosManuais(Carteira *carteira) {
    double soma;
    int i;

    do {
        limparTela();
        printf("=========== PESOS MANUAIS DA CARTEIRA ===========\n");
        printf("Digite os percentuais. A soma final deve ser 100%%.\n\n");

        for (i = 0; i < MAX_ATIVOS; i++) {
            char mensagem[130];
            snprintf(mensagem, sizeof(mensagem), "Percentual para %s: ", carteira->nomes[i]);
            carteira->pesos[i] = lerDoubleMinimo(mensagem, 0.0, 1);
        }

        soma = somarPesos(carteira);

        if (valorAbsoluto(soma - 100.0) > 0.01) {
            printf("\nEntrada invalida. A soma dos percentuais foi %.2f%%, mas precisa ser 100%%.\n", soma);
            printf("Digite novamente todos os pesos.\n");
            pausar();
        }
    } while (valorAbsoluto(soma - 100.0) > 0.01);
}

double somarPesos(const Carteira *carteira) {
    double soma = 0.0;
    int i;

    for (i = 0; i < MAX_ATIVOS; i++) {
        soma += carteira->pesos[i];
    }

    return soma;
}

void cadastrarCarteiraAtual(Carteira *carteira) {
    int i;
    double totalAtual = 0.0;

    limparTela();
    printf("=============== CARTEIRA ATUAL ===============\n");
    printf("Nesta parte, informe quanto dinheiro voce JA TEM em cada tipo de investimento.\n");
    printf("Nao precisa saber preco de cota, quantidade ou termos de corretora.\n");
    printf("Digite apenas o valor total em reais.\n\n");
    printf("Exemplos:\n");
    printf("- Tenho R$ 500 em Tesouro Selic: digite 500\n");
    printf("- Tenho R$ 120 em FIIs: digite 120\n");
    printf("- Ainda nao tenho esse investimento: digite 0\n\n");

    for (i = 0; i < MAX_ATIVOS; i++) {
        char mensagem[170];

        snprintf(mensagem, sizeof(mensagem), "Quanto voce ja tem em %s? R$ ", carteira->nomes[i]);
        carteira->valoresAtuais[i] = lerDoubleMinimo(mensagem, 0.0, 1);

        carteira->precos[i] = carteira->valoresAtuais[i];
        carteira->quantidades[i] = carteira->valoresAtuais[i] > 0.0 ? 1.0 : 0.0;
        totalAtual += carteira->valoresAtuais[i];
    }

    printf("\nCarteira atual cadastrada com sucesso.\n");
    printf("Total que voce ja possui investido: R$ %.2f\n", totalAtual);

    if (totalAtual == 0.0) {
        printf("Tudo bem se voce ainda nao tem investimentos. O simulador vai usar sua sobra mensal para sugerir os primeiros aportes.\n");
    } else {
        printf("Esses valores serao comparados com os percentuais da opcao 2 para mostrar o rebalanceamento.\n");
    }

    pausar();
}

void exibirResumo(const Orcamento *orcamento, const Carteira *carteira, const char categorias[MAX_GASTOS][30]) {
    int i;

    limparTela();
    printf("===================== RESUMO GERAL =====================\n");

    if (!orcamento->preenchido) {
        printf("Orcamento ainda nao cadastrado. Use a opcao 1 primeiro.\n");
        pausar();
        return;
    }

    printf("Usuario: %s\n", orcamento->nome);
    printf("Idade:          %d anos\n", orcamento->idade);
    printf("Renda mensal:       R$ %.2f\n", orcamento->rendaMensal);
    printf("Total de gastos:    R$ %.2f\n", orcamento->totalGastos);
    printf("Sobra para investir:");

    if (orcamento->sobraInvestir > 0.0) {
        printf(" R$ %.2f\n\n", orcamento->sobraInvestir);
    } else {
        printf(" R$ %.2f\n\n", orcamento->sobraInvestir);
        printf("Como nao houve sobra positiva, o ideal e ajustar o orcamento antes de investir.\n\n");
    }

    printf("Gastos por area:\n");
    for (i = 0; i < MAX_GASTOS; i++) {
        double percentual = 0.0;
        if (orcamento->rendaMensal > 0.0) {
            percentual = (orcamento->gastos[i] / orcamento->rendaMensal) * 100.0;
        }
        printf("- %-12s R$ %10.2f (%6.2f%% da renda)\n", categorias[i], orcamento->gastos[i], percentual);
    }

    if (orcamento->sobraInvestir <= 0.0) {
        pausar();
        return;
    }

    if (!carteira->preenchida || valorAbsoluto(somarPesos(carteira) - 100.0) > 0.01) {
        printf("\nPesos da carteira ainda nao cadastrados. Use a opcao 2.\n");
        pausar();
        return;
    }

    printf("\nSugestao simples para investir a sobra mensal:\n");
    for (i = 0; i < MAX_ATIVOS; i++) {
        double aporte = orcamento->sobraInvestir * carteira->pesos[i] / 100.0;
        printf("- %-58s R$ %10.2f (%5.2f%%)\n", carteira->nomes[i], aporte, carteira->pesos[i]);
    }

    simularRebalanceamento(orcamento, carteira);
    pausar();
}

void simularRebalanceamento(const Orcamento *orcamento, const Carteira *carteira) {
    double totalAtual = 0.0;
    double totalProjetado;
    int i;

    for (i = 0; i < MAX_ATIVOS; i++) {
        totalAtual += carteira->valoresAtuais[i];
    }

    if (totalAtual <= 0.0) {
        printf("\nVoce ainda nao cadastrou valores atuais na carteira.\n");
        printf("Mesmo assim, a divisao acima ja mostra quanto investir em cada item por mes.\n");
        return;
    }

    totalProjetado = totalAtual + orcamento->sobraInvestir;

    printf("\n================ REBALANCEAMENTO ================\n");
    printf("Carteira atual: R$ %.2f\n", totalAtual);
    printf("Carteira depois do novo aporte: R$ %.2f\n\n", totalProjetado);
    printf("%-58s %12s %12s %18s\n", "Investimento", "Atual", "Ideal", "Acao sugerida");
    printf("------------------------------------------------------------------------------------------------\n");

    for (i = 0; i < MAX_ATIVOS; i++) {
        double valorIdeal = totalProjetado * carteira->pesos[i] / 100.0;
        double diferenca = valorIdeal - carteira->valoresAtuais[i];

        printf("%-58s R$ %9.2f R$ %9.2f ", carteira->nomes[i], carteira->valoresAtuais[i], valorIdeal);

        if (diferenca > 0.01) {
            printf("Aportar R$ %.2f\n", diferenca);
        } else if (diferenca < -0.01) {
            printf("Pausar aporte. Excesso de R$ %.2f\n", valorAbsoluto(diferenca));
        } else {
            printf("Manter\n");
        }
    }

    printf("\nLeitura para leigos:\n");
    printf("- Aportar significa colocar mais dinheiro no item que esta abaixo da meta.\n");
    printf("- Pausar aporte significa que o item esta acima da meta. Iniciantes podem apenas nao colocar dinheiro nele por enquanto.\n");
    printf("- Nunca invista dinheiro da reserva de emergencia em produtos de alto risco.\n");
}

void exibirAjudaInvestimentos(void) {
    limparTela();
    printf("================ AJUDA PARA LEIGOS ================\n");
    printf("Este programa nao substitui um profissional, mas ajuda a organizar ideias.\n\n");

    printf("1. Reserva de emergencia\n");
    printf("   Primeiro passo. Deve ficar em algo seguro e com liquidez diaria.\n");
    printf("   Exemplos: Tesouro Selic, CDB com liquidez diaria e fundo DI simples.\n");
    printf("   Objetivo: cobrir de 3 a 6 meses de gastos essenciais.\n\n");

    printf("2. Renda fixa conservadora\n");
    printf("   Boa para quem esta comecando e quer previsibilidade.\n");
    printf("   Exemplos: CDB, LCI, LCA e Tesouro Selic. Compare rentabilidade, prazo e garantia do FGC.\n\n");

    printf("3. Tesouro IPCA+\n");
    printf("   Ajuda a proteger o dinheiro da inflacao no longo prazo.\n");
    printf("   Pode oscilar antes do vencimento, entao combina melhor com objetivos distantes.\n\n");

    printf("4. Fundos imobiliarios (FIIs)\n");
    printf("   Sao negociados na bolsa e podem pagar rendimentos mensais.\n");
    printf("   Possuem risco de mercado, vacancia, gestao e mudanca nos rendimentos.\n\n");

    printf("5. ETFs de acoes\n");
    printf("   Sao fundos negociados em bolsa que compram varias acoes de uma vez.\n");
    printf("   Servem para diversificar sem escolher empresa por empresa, mas oscilam bastante.\n\n");

    printf("Regra simples para iniciantes:\n");
    printf("- Quite dividas caras antes de investir.\n");
    printf("- Monte reserva de emergencia antes de correr risco.\n");
    printf("- Invista todo mes, mesmo pouco.\n");
    printf("- Rebalanceie quando um investimento ficar muito acima ou abaixo da meta.\n");
    printf("- Desconfie de promessa de ganho rapido e garantido.\n");

    pausar();
}

double valorAbsoluto(double valor) {
    if (valor < 0.0) {
        return -valor;
    }

    return valor;
}