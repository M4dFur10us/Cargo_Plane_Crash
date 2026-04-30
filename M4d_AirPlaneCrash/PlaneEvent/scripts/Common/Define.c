// =====================================================================================
// Define.c - VERSÃO DEFINITIVA (Disjuntores de Compilação)
// Responsabilidade: Declarar as flags globais do pré-processador antes do carregamento.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// DIRETIVAS GLOBAIS DE PRÉ-PROCESSADOR
	// ---------------------------------------------------------------------------------
	// Estas flags são lidas pela engine do DayZ exclusivamente em tempo de compilação
	// (quando o servidor liga). Elas não consomem memória RAM em runtime.
	// 
	// PLANEEVENT: Identifica para todo o ecossistema que o mod está ativo.
	// M4D_DISABLE_LEGACY_DUPLICATE: Kill Switch que desativa códigos antigos conflitantes.
	// ---------------------------------------------------------------------------------

#define PLANEEVENT
#define M4D_DISABLE_LEGACY_DUPLICATE