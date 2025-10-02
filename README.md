# 🌻 projeto girassol - rastreador solar simulado

este projeto é a entrega do **projeto 1 da parte 2** da disciplina de controle e automação.  
o objetivo foi simular um rastreador solar que movimenta um painel entre -90° e +90° para seguir a maior intensidade de luz.

## componentes simulados
- arduino uno  
- 2 fotoresistores (ldr) para detectar a luz esquerda/direita  
- potenciômetro simulando o encoder (posição angular)  
- 2 push buttons representando os fins de curso (-90° e +90°)  
- 2 leds (verde e vermelho) representando o motor girando para cada lado  

## como funciona
- os dois ldres medem a luz e geram uma referência angular  
- o potenciômetro funciona como a posição real do painel  
- um controlador pid calcula o erro entre referência e medido  
- os leds acendem indicando o sentido de giro do motor (verde = direita, vermelho = esquerda)  
- os botões cortam o movimento quando o painel chega nos limites físicos  

## arquivos
- `sketch.ino` → código principal do arduino  
- `diagram.json` → esquema de ligação no wokwi  
- `docs/documentacao.md` → descrição dos pinos e montagem  
- `docs/relatorio.md` → relatório com resultados e análise  

## demonstração
a simulação foi feita no [wokwi](https://wokwi.com).  
na apresentação será mostrado:  
- leds acendendo conforme mudança da luz nos ldres  
- logs no serial monitor com ângulo de referência, posição medida, erro e pwm  
- funcionamento dos fins de curso  

