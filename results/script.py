import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv('dungeon.csv')

df['celulas_uteis'] = (df['dimensao'] ** 2) - 2
df['densidade_pct'] = ((df['buracos'] + df['monstros']) / df['celulas_uteis']) * 100


def categorizar_densidade(pct):
    if pct <= 10: return '1. Ultra-Baixa (Até 10%)'
    elif pct <= 20: return '2. Baixa (11% a 20%)'
    elif pct <= 30: return '3. Moderada (21% a 30%)'
    elif pct <= 40: return '4. Alta (31% a 40%)'
    else: return '5. Crítica (40%+)'

df['faixa_densidade'] = df['densidade_pct'].apply(categorizar_densidade)



print("--- GERANDO TABELA DE LETALIDADE E DESEMPENHO ---")
tabela_resumo = df.groupby('faixa_densidade').agg({
    'taxa_vitoria': 'mean',
    'morte_buraco': 'mean',
    'morte_monstro': 'mean',
    'travamento': 'mean',
    'score_medio': 'mean',
    'desvio_padrao': 'mean'
}).rename(columns={
    'taxa_vitoria': 'Vitória Média (%)',
    'morte_buraco': 'Morte Buraco (%)',
    'morte_monstro': 'Morte Monstro (%)',
    'travamento': 'Travado (%)',
    'score_medio': 'Score Médio',
    'desvio_padrao': 'Desvio Padrão Médio'
})


tabela_resumo.to_csv('csv/resultados_agrupados.csv')
print(tabela_resumo.round(2))
print("\n" + "="*50 + "\n")

sns.set_theme(style="whitegrid")



plt.figure(figsize=(14, 8))

sns.lineplot(data=df, x='densidade_pct', y='score_medio', hue='dimensao', 
             palette='tab10', marker='o', linewidth=2)

plt.title('Impacto da Densidade Combinada de Perigos na Pontuação do Agente', fontsize=14, pad=15)
plt.xlabel('Densidade de Perigos (%)', fontsize=12)
plt.ylabel('Pontuação Média', fontsize=12)
plt.legend(title='Dimensão da Sala', bbox_to_anchor=(1.05, 1), loc='upper left')
plt.tight_layout()
plt.savefig('img/score.png', dpi=400)
plt.close()



dimensoes_unicas = sorted(df['dimensao'].unique())

for dim in dimensoes_unicas:
    df_dim = df[df['dimensao'] == dim]

    matrix_vitoria = df_dim.pivot(index='monstros', columns='buracos', values='taxa_vitoria')
    
    plt.figure(figsize=(14, 8))
    sns.heatmap(matrix_vitoria, annot=True, fmt=".1f", cmap="RdYlGn", 
                cbar_kws={'label': 'Taxa de Vitória (%)'}, vmin=0, vmax=100)
    
    plt.title(f'Mapa de Calor de Vitórias - Dungeon {dim}x{dim}', fontsize=14, pad=15)
    plt.xlabel('Quantidade de Buracos', fontsize=12)
    plt.ylabel('Quantidade de Monstros', fontsize=12)
    plt.gca().invert_yaxis() # Inverte o eixo Y para o 1 ficar embaixo
    plt.tight_layout()
    plt.savefig(f'img/vitorias_{dim}x{dim}.png', dpi=400)
    plt.close()

print("Todos os gráficos e tabelas foram gerados com sucesso!")