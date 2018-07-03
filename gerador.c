/* 
 * TÍTULO: Código C para gerar hashes DES e MD5
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <crypt.h>
/* Este codigo precisa ser compilado no Linux da seguinte forma
 *
 *     gcc hashLinux.c -lcrypt -o hashLinux
 *
 * E executado:
 *    ./hashLinux
 *
 * */

/* 
 * Existe uma função na lib do Linux para gerar hahes, a crypt. Basta fornecer
 * o salt number e a senha que ela retorna o hash
 *
 * A lógica para geração de hahes é a seguinte:
 *  - usuário digita uma senha
 *  - sistema sorteia os caracteres de salt number 
 *  - gera-se o hash correspondente
 * 
 * Para entender o que é salt number e a sua extrema relevância, leia o artigo
 * http://www.vivaolinux.com.br/artigo/Armazenamento-de-senhas-no-Linux/
 *
 * Veja que facilmente poderia-se converter este programa para
 * verificar se a senha confere. Apenas NãO TERIA a etapa de sortear o salt number
 *
 * Para verificar se uma senha digitada confere as etapas seriam estas:
 *  - usuário digita seu login
 *  - pelo login, o programa já vai no /etc/shadow e pega o hash deste login, 
 *    digamos, dG5OdAa8n/36k no DES ou $1$23OEoGVk$4jcc8phmHcGv.dpNkBGtu0 no MD5
 * 
 *  - se for DES, o sistema pega o sal number que está no shadow (dG do dG5OdAa8n/36k)
 *
 *  - chama a função crypt(senhaFornecida, "dG") e verifica se o resultado gerado
 *    bate com "dG5OdAa8n/36k". Beteu? Senha confere
 *    
 *  - Se for MD5, a mesma coisa, pega o salt number do hash do arquivo /etc/shadow
 *    (exemplo: $1$23OEoGVk$4jcc8phmHcGv.dpNkBGtu0 ==> salt = "$1$23OEoGVk")
 *    
 *  - chama a função crypt(senhaFornecida, "$1$23OEoGVk") e verifica se o resultado
 *    bate com $1$23OEoGVk$4jcc8phmHcGv.dpNkBGtu0 Bateu? Senha confere.
 **/
int main()
{
    /* Na hora de sortear os caracteres de salt number, apenas os cars válidos serão usados */
    char saltValidos[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuwxyz0123456789./";
    char saltValidosTam = strlen(saltValidos);
    char senhaTextoClaro[30];
    char *senhaHash;
    char salt[15];
    char spos, i;

    /* Disparando uma semente de números randômicos */
    srand(time(NULL));
    printf("Digite uma senha:\n");
    scanf("%30s", senhaTextoClaro);

/* O Unix historicamente baseia o seu HASH no método DES, onde o mesmo é convertido para atuar
 * como hash. No DES o salt number é composto por DOIS caracteres.
 * (detalhes avançados de como é composto o hash DES e também o MD5 estão no artigo já citado
 * e em um email técnico meu que se encontra publicado em http://gravatai.ulbra.tche.br/~elgio/senhas.html
 *
 * 1) Gerando o hash DES:
 **/

    /* Sorteando aleatoriamente um caractere do salt 
     * técnica usada: sortei-se um inteiro entre 0 e saltValidosTam (quantidade de cars no vetor
     * saltValidos) este inteiro será o índice do vetor de cars 
    */
    spos = random() % saltValidosTam;
    salt[0] = saltValidos[spos];

    srand(random()); // regerando a semente, para evitar vícios da randomicidade

    /* Sorteando aleatoriamente outro caractere do salt */
    spos = random() % saltValidosTam;

    salt[1] = saltValidos[spos];
    salt[2] = 0;      // fechando a string

    printf("Acabei escolhendo %s como salt number para o DES\n", salt);
    senhaHash = (char *) crypt(senhaTextoClaro, salt);
    printf("Hash Unix DES da senha = %s\n", senhaHash);
    printf("Um Unix antigo armazenaria o hash %s para este usuario. ", senhaHash);
    printf("Observe que as duas primeiras letras do hash sao o salt escolhido\n");

/* Usar o hash DES para senhas tem sérios problemas, pois na verdade não é necessário testar 2^56 
 * para quebrar. Isto porque neste sistema antigo, uma senha tinha no máximo 8 caracteres, ou seja
 * 56 bits (pois o último bit de cada car é descartado), que eram usado como bloco de entrada. 
 * Porém como o usuário só pode digitar letras, números e símbolos, o número de tentativas é bem
 * menos do que isto. Seria impossivel, por exemplo, a senha ser totalmente preenchida com bits 0!
 *
 *
 * 2) Gerando o hash MD5:
 * O crypt nunca foi preparado para lidar com MD5. Um paleativo foi colocado na lib: se o salt 
 * number for de DOIS caracteres, é um hash DES. Se o salt number COMEÇAR com $1$, entao é MD5.
 * Assim mantém-se a compatibilidade. (outras versões de hash foram gradativamente introduzidas, 
 * como o SHA1)
 *
 * Preciso passar para a função crypt, portanto, $1$<salt escolhido> para usar MD5
 *
 **/
    /* Sorteando aleatoriamente cars  do salt */
    salt[0] = '$';
    salt[1] = '1';
    salt[2] = '$';
    for (i = 0; i < 8; i++) {
   spos = random() % saltValidosTam;
   salt[i + 3] = saltValidos[spos];
   salt[i + 4] = 0;   // fechando a string
   srand(random());
    }

    printf("Acabei escolhendo %s como salt number para o MD5\n", salt);
    senhaHash = (char *) crypt(senhaTextoClaro, salt);
    printf("Hash Unix MD5 da senha = %s\n", senhaHash);
    printf("Um Unix novo armazenaria o hash %s para este usuario\n", senhaHash);
    printf("Observe que o $1$ indica ser MD5. O que vem depois ate o proximo $ eh o salt escolhido\n");

    return(0);
}

//gcc  gerador.c -lcrypt -o gerador
