#include "relationshipwidget.h"
#include "restricaowidget.h"
#include "columnwidget.h"
#include "tabelawidget.h"
#include "messagebox.h"

extern RestricaoWidget *restricao_wgt;
extern ColumnWidget *coluna_wgt;
extern TabelaWidget *tabela_wgt;
extern MessageBox *caixa_msg;

RelationshipWidget::RelationshipWidget(QWidget *parent): BaseObjectWidget(parent, OBJ_RELATIONSHIP)
{
	try
	{
		QStringList list;
		QGridLayout *grid=NULL;
		QFrame *frame=NULL;

		Ui_RelationshipWidget::setupUi(this);
		//Alocando e configurando os destcadores de nomes das tabelas
		table1_hl=NULL;
		operation_count=0;
		table1_hl=new SyntaxHighlighter(ref_table_txt, false);
		table1_hl->loadConfiguration(GlobalAttributes::CONFIGURATIONS_DIR +
																		 GlobalAttributes::DIR_SEPARATOR +
																		 GlobalAttributes::SQL_HIGHLIGHT_CONF +
																		 GlobalAttributes::CONFIGURATION_EXT);

		table2_hl=NULL;
		table2_hl=new SyntaxHighlighter(recv_table_txt, false);
		table2_hl->loadConfiguration(GlobalAttributes::CONFIGURATIONS_DIR +
																		 GlobalAttributes::DIR_SEPARATOR +
																		 GlobalAttributes::SQL_HIGHLIGHT_CONF +
																		 GlobalAttributes::CONFIGURATION_EXT);

		//Alocando as tabela de atributos e restrições do relacionamento
		attributes_tab=new TabelaObjetosWidget(TabelaObjetosWidget::TODOS_BOTOES ^
																					(TabelaObjetosWidget::BTN_ATUALIZAR_ITEM |
																					 TabelaObjetosWidget::BTN_MOVER_ITENS), true, this);

		constraints_tab=new TabelaObjetosWidget(TabelaObjetosWidget::TODOS_BOTOES  ^
																					 (TabelaObjetosWidget::BTN_ATUALIZAR_ITEM |
																						TabelaObjetosWidget::BTN_MOVER_ITENS), true, this);

		advanced_objs_tab=new TabelaObjetosWidget(TabelaObjetosWidget::BTN_EDITAR_ITEM, true, this);

		//Configurando os rótulos e ícones das colunas das tabelas
		attributes_tab->definirNumColunas(2);
		attributes_tab->definirRotuloCabecalho(trUtf8("Attribute"), 0);
		attributes_tab->definirIconeCabecalho(QPixmap(":/icones/icones/column.png"),0);
		attributes_tab->definirRotuloCabecalho(trUtf8("Type"), 1);
		attributes_tab->definirIconeCabecalho(QPixmap(":/icones/icones/usertype.png"),1);

		constraints_tab->definirNumColunas(2);
		constraints_tab->definirRotuloCabecalho(trUtf8("Constraint"), 0);
		constraints_tab->definirIconeCabecalho(QPixmap(":/icones/icones/constraint.png"),0);
		constraints_tab->definirRotuloCabecalho(trUtf8("Type"), 1);
		constraints_tab->definirIconeCabecalho(QPixmap(":/icones/icones/usertype.png"),1);

		advanced_objs_tab->definirNumColunas(2);
		advanced_objs_tab->definirRotuloCabecalho(trUtf8("Name"), 0);
		advanced_objs_tab->definirIconeCabecalho(QPixmap(":/icones/icones/column.png"),0);
		advanced_objs_tab->definirRotuloCabecalho(trUtf8("Type"), 1);
		advanced_objs_tab->definirIconeCabecalho(QPixmap(":/icones/icones/usertype.png"),1);

		connect(advanced_objs_tab, SIGNAL(s_linhaEditada(int)), this, SLOT(showAdvancedObject(int)));

		//Adiciona as tabelas alocadas  s respectivas abas
		grid=new QGridLayout;
		grid->addWidget(attributes_tab, 0,0,1,1);
		grid->setContentsMargins(2,2,2,2);
		rel_attribs_tbw->widget(1)->setLayout(grid);

		grid=new QGridLayout;
		grid->addWidget(constraints_tab, 0,0,1,1);
		grid->setContentsMargins(2,2,2,2);
		rel_attribs_tbw->widget(2)->setLayout(grid);

		grid=dynamic_cast<QGridLayout *>(rel_attribs_tbw->widget(0)->layout());
		//Gera um frame de alerta sobre a edição de atributos do relacionamento
		frame=generateInformationFrame(trUtf8("Editing attributes of an existing relationship is allowed, but must be done carefully because it may break references to columns and cause invalidation of objects such as triggers, indexes, constraints and sequences."));
		grid->addWidget(frame, grid->count()+1, 0, 1, 3);
		frame->setParent(rel_attribs_tbw->widget(0));

		grid=dynamic_cast<QGridLayout *>(rel_attribs_tbw->widget(3)->layout());
		//Gera um frame de informação sobre a criação de chave primária especial
		frame=generateInformationFrame(trUtf8("Use the special primary key if you want to include a primary key containing inherited / copied columns to the receiving table. This is a feature available only for generalization / copy relationships."));

		grid->addWidget(frame, 1, 0, 1, 1);
		frame->setParent(rel_attribs_tbw->widget(3));

		grid=new QGridLayout;
		grid->setContentsMargins(2,2,2,2);

		grid->addWidget(advanced_objs_tab, 0, 0, 1, 1);

		//Gera um frame de informação sobre a aba avançada
		frame=generateInformationFrame(trUtf8("This advanced tab shows the objects (columns or table) auto created by the relationship's connection as well the foreign keys that represents the link between the participant tables."));
		grid->addWidget(frame, 1, 0, 1, 1);

		rel_attribs_tbw->widget(4)->setLayout(grid);

		configureFormLayout(relationship_grid, OBJ_RELATIONSHIP);
		parent_form->setMinimumSize(600, 520);

		//Configurando o combo de tipo de postergação com os tipos disponíveis
		DeferralType::getTypes(list);
		deferral_cmb->addItems(list);

		connect(parent_form->aplicar_ok_btn,SIGNAL(clicked(bool)), this, SLOT(applyConfiguration(void)));
		connect(parent_form->cancelar_btn,SIGNAL(clicked(bool)), this, SLOT(cancelConfiguration(void)));
		connect(deferrable_chk, SIGNAL(toggled(bool)), deferral_cmb, SLOT(setEnabled(bool)));
		connect(deferrable_chk, SIGNAL(toggled(bool)), deferral_lbl, SLOT(setEnabled(bool)));

		connect(auto_suffix_chk, SIGNAL(toggled(bool)), src_suffix_lbl, SLOT(setDisabled(bool)));
		connect(auto_suffix_chk, SIGNAL(toggled(bool)), src_suffix_edt, SLOT(setDisabled(bool)));
		connect(auto_suffix_chk, SIGNAL(toggled(bool)), dst_suffix_lbl, SLOT(setDisabled(bool)));
		connect(auto_suffix_chk, SIGNAL(toggled(bool)), dst_suffix_edt, SLOT(setDisabled(bool)));

		connect(identifier_chk, SIGNAL(toggled(bool)), table1_mand_chk, SLOT(setDisabled(bool)));
		connect(identifier_chk, SIGNAL(toggled(bool)), table2_mand_chk, SLOT(setDisabled(bool)));

		connect(attributes_tab, SIGNAL(s_linhasRemovidas(void)), this, SLOT(removeObjects(void)));
		connect(attributes_tab, SIGNAL(s_linhaAdicionada(int)), this, SLOT(addObject(void)));
		connect(attributes_tab, SIGNAL(s_linhaEditada(int)), this, SLOT(editObject(int)));
		connect(attributes_tab, SIGNAL(s_linhaRemovida(int)), this, SLOT(removeObject(int)));

		connect(constraints_tab, SIGNAL(s_linhasRemovidas(void)), this, SLOT(removeObjects(void)));
		connect(constraints_tab, SIGNAL(s_linhaAdicionada(int)), this, SLOT(addObject(void)));
		connect(constraints_tab, SIGNAL(s_linhaEditada(int)), this, SLOT(editObject(int)));
		connect(constraints_tab, SIGNAL(s_linhaRemovida(int)), this, SLOT(removeObject(int)));

	}
	catch(Exception &e)
	{
		//Redireciona o erro
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void RelationshipWidget::hideEvent(QHideEvent *event)
{
	BaseRelationship *rel=dynamic_cast<BaseRelationship *>(this->object);

	identifier_chk->setChecked(false);
	table1_mand_chk->setChecked(false);
	table2_mand_chk->setChecked(false);
	relnn_tab_name_edt->clear();
	dst_suffix_edt->clear();
	src_suffix_edt->clear();
	deferrable_chk->setChecked(false);
	deferral_cmb->setCurrentIndex(0);
	rel_attribs_tbw->setCurrentIndex(0);

	attributes_tab->blockSignals(true);
	constraints_tab->blockSignals(true);
	attributes_tab->removerLinhas();
	constraints_tab->removerLinhas();
	attributes_tab->blockSignals(false);
	constraints_tab->blockSignals(false);

	rel_columns_lst->clear();

	/* Caso o objeto seja novo e o usuário fecha a janela sem aplicar
		 as configurações o mesmo será destruído */
	if(rel && !rel->isModified())
	{
		this->cancelConfiguration();

		/*if(this->novo_obj)
	{
	 delete(this->objeto);
	 this->objeto=NULL;
	}*/
	}

	BaseObjectWidget::hideEvent(event);
}

void RelationshipWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Table *src_tab, Table *dst_tab, unsigned rel_type)
{
	Relationship *rel=NULL;

	try
	{
		//QString nome;

		//Cria um nome temporário para o relacionamento
		//nome=QString("rel_") + tab_orig->getName() + QString("_") + tab_dest->getName();

		//Aloca o novo relacionamento
		//rel=new Relacionamento(nome, tipo_rel, tab_orig, tab_dest);

		//if(tipo_rel==BaseRelationship::RELATIONSHIP_GEN ||
		//	 tipo_rel==BaseRelationship::RELATIONSHIP_DEP ||)
		//	rel=new Relationship(tipo_rel, tab_dest, tab_orig);
		//else
			rel=new Relationship(rel_type, src_tab, dst_tab);


		/* Marca como novo objeto o relacionamento gerado, assim o mesmo é tratado
		 de forma diferente nos métodos de configuração da classe superior */
		this->new_object=true;

		/* Inicia o encademanento de operações, assim todo objeto editado dentro
		 do relacionameto será encadeado na lista, desta forma quando o usuário
		 necessitar desfazer as modificações do relacionamentos, os objetos do
		 relacionemento também serão restaurados */
		op_list->startOperationChain();

		operation_count=op_list->getCurrentSize();

		//Adiciona o relacionamento criado   lista de operações
		op_list->registerObject(rel, Operation::OBJECT_CREATED);

		//Chama o método publico de definição dos atributos
		this->setAttributes(model, op_list, rel);
	}
	catch(Exception &e)
	{
		op_list->removeLastOperation();
		//if(rel) delete(rel);
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void RelationshipWidget::setAttributes(DatabaseModel *model, OperationList *op_list, BaseRelationship *base_rel)
{
	static QWidget *tabs[4]={ rel_attribs_tbw->widget(1), rel_attribs_tbw->widget(2), rel_attribs_tbw->widget(3), rel_attribs_tbw->widget(4) };
	static QString tab_lables[4]={ rel_attribs_tbw->tabText(1), rel_attribs_tbw->tabText(2), rel_attribs_tbw->tabText(3), rel_attribs_tbw->tabText(4)};
	unsigned rel_type, i;
	Relationship *aux_rel=NULL;
	bool rel1n, relnn, relgen_dep;

	//Caso o relacionamento não esteja alocado dispara um erro
	if(!base_rel)
		throw Exception(ERR_ASG_NOT_ALOC_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	//Define os atributos do formulários e da janela pai
	BaseObjectWidget::setAttributes(model, op_list, base_rel);

	/* Inicia o encademanento de operações, assim todo objeto editado dentro
		do relacionameto será encadeado na lista, desta forma quando o usuário
		necessitar desfazer as modificações do relacionamentos, os objetos do
		relacionemento também serão restaurados */
	if(!this->new_object)
	{
		op_list->startOperationChain();
		operation_count=op_list->getCurrentSize();
	}

	rel_type=base_rel->getRelationshipType();

	//Marcado o radiobox que indica o tipo do relacionamento
	switch(rel_type)
	{
		case BaseRelationship::RELATIONSHIP_11: rel_11_rb->setChecked(true); break;
		case BaseRelationship::RELATIONSHIP_1N: rel_1n_rb->setChecked(true); break;
		case BaseRelationship::RELATIONSHIP_NN: rel_nn_rb->setChecked(true); break;
		case BaseRelationship::RELATIONSHIP_GEN: rel_gen_rb->setChecked(true); break;
		case BaseRelationship::RELATIONSHIP_FK:  rel_fk_rb->setChecked(true); break;
		case BaseRelationship::RELATIONSHIP_DEP: rel_dep_rb->setChecked(true); break;
	}

	//Converte o objeto para relacionamento entre tabelas para acessar os atributos
	aux_rel=dynamic_cast<Relationship *>(base_rel);

	if(base_rel->getObjectType()==BASE_RELATIONSHIP ||
		 (aux_rel && aux_rel->getRelationshipType()==BaseRelationship::RELATIONSHIP_NN))
	{
		ref_table_lbl->setText(trUtf8("Table 1:"));
		ref_table_txt->setPlainText(QString::fromUtf8(base_rel->getTable(BaseRelationship::SRC_TABLE)->getName(true)));
		recv_table_lbl->setText(trUtf8("Table 2:"));
		recv_table_txt->setPlainText(QString::fromUtf8(base_rel->getTable(BaseRelationship::DST_TABLE)->getName(true)));
	}

	table1_mand_chk->setText(QString::fromUtf8(base_rel->getTable(BaseRelationship::SRC_TABLE)->getName()) + trUtf8(" is required"));
	table2_mand_chk->setText(QString::fromUtf8(base_rel->getTable(BaseRelationship::DST_TABLE)->getName()) + trUtf8(" is required"));

	//Caso o relacionamento seja entre tabelas
	if(aux_rel)
	{
		//vector<QString> vet_cols;
		vector<Column *> cols;
		vector<unsigned> col_ids;
		int count, i;
		QListWidgetItem *item=NULL;

		//Exibe o nome das tabelas participantes relacionamento no formulário
		if(rel_type!=BaseRelationship::RELATIONSHIP_NN)
		{
			ref_table_lbl->setText(trUtf8("Reference Table:"));
			ref_table_txt->setPlainText(QString::fromUtf8(aux_rel->getReferenceTable()->getName(true)));
			recv_table_lbl->setText(trUtf8("Receiver Table:"));
			recv_table_txt->setPlainText(QString::fromUtf8(aux_rel->getReceiverTable()->getName(true)));
		}

		//Preenche os campos do formulário com os valores presentes no relacionamento
		auto_suffix_chk->setChecked(aux_rel->isAutomaticSuffix());

		if(!auto_suffix_chk->isChecked())
		{
			src_suffix_edt->setText(QString::fromUtf8(aux_rel->getTableSuffix(BaseRelationship::SRC_TABLE)));
			dst_suffix_edt->setText(QString::fromUtf8(aux_rel->getTableSuffix(BaseRelationship::DST_TABLE)));
		}

		table1_mand_chk->setChecked(aux_rel->isTableMandatory(BaseRelationship::SRC_TABLE));
		table2_mand_chk->setChecked(aux_rel->isTableMandatory(BaseRelationship::DST_TABLE));
		identifier_chk->setChecked(aux_rel->isIdentifier());
		deferrable_chk->setChecked(aux_rel->isDeferrable());
		relnn_tab_name_edt->setText(aux_rel->getTableNameRelNN());

		//Habilita os botões das tabelas de restições e atributos caso o relacionamento esteja protegido
		attributes_tab->habilitarBotoes(TabelaObjetosWidget::TODOS_BOTOES, !aux_rel->isProtected());
		constraints_tab->habilitarBotoes(TabelaObjetosWidget::TODOS_BOTOES, !aux_rel->isProtected());

		//Lista as restrições e atributos do relacionamento
		listObjects(OBJ_COLUMN);
		listObjects(OBJ_CONSTRAINT);

		/* Caso seja um novo objeto é necessário conectar o relacionamento para que
		 as colunas sejam criadas na tabela receptora e seus nomes obtidos
		 para listagem no campo "chave primária" */
		if(rel_type==BaseRelationship::RELATIONSHIP_GEN ||
			 rel_type==BaseRelationship::RELATIONSHIP_DEP)
		{
			if(this->new_object)
			{
				aux_rel->connectRelationship();
				//vet_cols=relacao_aux->getRelationshipColumnsNames();
				cols=aux_rel->getGeneratedColumns();
			}
			else
				//vet_cols=relacao_aux->getRelationshipColumnsNames();
				cols=aux_rel->getGeneratedColumns();

			//Obtém os índices das colunas da chave primária especial no relacionamento
			col_ids=aux_rel->getSpecialPrimaryKeyCols();

			//Lista os nomes da colunas criadas pelo relacionamento
			count=cols.size();
			for(i=0; i < count; i++)
			{
				rel_columns_lst->addItem(cols[i]->getName().toUtf8() +
																" (" + QString::fromUtf8(*cols[i]->getType()) + ")");
				item=rel_columns_lst->item(i);
				item->setCheckState(Qt::Unchecked);
			}

			/* Marca na lista de colunas da chave primária especial os itens
			conforme os ids vindos do relacionamento */
			count=col_ids.size();
			for(i=0; i < count; i++)
			{
				if(col_ids[i] < static_cast<unsigned>(rel_columns_lst->count()))
					rel_columns_lst->item(col_ids[i])->setCheckState(Qt::Checked);
			}

			if(this->new_object)
				aux_rel->disconnectRelationship();
		}
	}

	//Configurando quais objetos do formulário devem ser exibidos conforme o tipo do relacionamento
	rel1n=(rel_type==BaseRelationship::RELATIONSHIP_11 ||
				 rel_type==BaseRelationship::RELATIONSHIP_1N);

	relnn=(rel_type==BaseRelationship::RELATIONSHIP_NN);

	relgen_dep=(rel_type==BaseRelationship::RELATIONSHIP_DEP ||
							rel_type==BaseRelationship::RELATIONSHIP_GEN ||
							rel_type==BaseRelationship::RELATIONSHIP_FK);

	//Sufixo de origem: exibido para 1-n ou n-n
	src_suffix_lbl->setVisible(rel1n || relnn);
	//src_suffix_lbl->setEnabled(rel1n || relnn);
	src_suffix_edt->setVisible(rel1n || relnn);
	//src_suffix_edt->setEnabled(rel1n || relnn);

	//Sufixo de destino: exibido para n-n
	dst_suffix_lbl->setVisible(rel1n || relnn);
	//dst_suffix_lbl->setEnabled(relnn);
	dst_suffix_edt->setVisible(rel1n || relnn);
	//dst_suffix_edt->setEnabled(relnn);

	auto_suffix_chk->setVisible(rel1n || relnn);

	//Obrigatoriedade de tabela de origem: exibido para 1-n e n-n
	card_lbl->setVisible(rel1n);
	table1_mand_chk->setEnabled(rel1n);
	table1_mand_chk->setVisible(rel1n);
	//Obrigatoriedade de tabela de destino: exibido para 1-1 e n-n
	table2_mand_chk->setEnabled(rel_type==BaseRelationship::RELATIONSHIP_11);
	table2_mand_chk->setVisible(rel1n);

	/* Rel. Identificador: exibido para 1-n E quando as tabelas participantes
		são diferentes (não é autorelacionamento) */
	identifier_chk->setVisible(rel1n &&
																(base_rel->getTable(BaseRelationship::SRC_TABLE) !=
			base_rel->getTable(BaseRelationship::DST_TABLE)));

	//Postergação de restrição: exibido para 1-n
	/*postergavel_lbl->setVisible(rel1n);
 deferrable_chk->setVisible(rel1n);
 deferral_lbl->setVisible(rel1n);
 deferral_cmb->setVisible(rel1n);*/
	foreign_key_gb->setVisible(rel1n);

	//Obrigatoriedade de tabelas: exibido para 1-n
	relnn_tab_name_lbl->setVisible(relnn);
	relnn_tab_name_edt->setVisible(relnn);

	for(i=0; i < 4; i++)
		rel_attribs_tbw->removeTab(1);

	if(!relgen_dep)
	{
		for(i=0; i < 2; i++)
			rel_attribs_tbw->addTab(tabs[i], tab_lables[i]);
	}
	else if(relgen_dep && base_rel->getObjectType()==OBJ_RELATIONSHIP)
		rel_attribs_tbw->addTab(tabs[2], tab_lables[2]);

	if(base_rel->getObjectType()==OBJ_RELATIONSHIP ||
		 (base_rel->getObjectType()==BASE_RELATIONSHIP &&
			base_rel->getRelationshipType()==BaseRelationship::RELATIONSHIP_FK))
		rel_attribs_tbw->addTab(tabs[3], tab_lables[3]);


	listAdvancedObjects();
}

void RelationshipWidget::listObjects(ObjectType obj_type)
{
	TabelaObjetosWidget *tab=NULL;
	Relationship *rel=NULL;
	unsigned count, i;

	try
	{
		//Seleciona a tabela de objetos de acordo com o tipo especificado
		if(obj_type==OBJ_COLUMN)
			tab=attributes_tab;
		else
			tab=constraints_tab;

		//Obtém a referência ao relacionamento
		rel=dynamic_cast<Relationship *>(this->object);

		//Remove as linhas da tabela antes da exibição dos elementos
		tab->blockSignals(true);
		tab->removerLinhas();

		//Obtém a quantidade de elementos a serem exibidos
		count=rel->getObjectCount(obj_type);
		for(i=0; i < count; i++)
		{
			//Adicionar uma linha
			tab->adicionarLinha();
			//Exibe o objeto atual na linha atual da tabela
			showObjectData(rel->getObject(i, obj_type), i);
		}
		tab->limparSelecao();
		tab->blockSignals(false);

		//Habilita o botão de inserção de restrições somente quando há atributos no relacionamento
		constraints_tab->habilitarBotoes(TabelaObjetosWidget::BTN_INSERIR_ITEM,
																		attributes_tab->obterNumLinhas() > 0);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void RelationshipWidget::listAdvancedObjects(void)
{
	BaseRelationship *base_rel=NULL;
	Relationship *rel=NULL;
	Table *tab=NULL;
	vector<Column *> cols;
	vector<Constraint *> constrs;
	unsigned count=0, i,i1;

	try
	{
		//Obtém a referência ao relacionamento
		base_rel=dynamic_cast<BaseRelationship *>(this->object);
		rel=dynamic_cast<Relationship *>(base_rel);

		//Remove as linhas da tabela antes da exibição dos elementos
		advanced_objs_tab->blockSignals(true);
		advanced_objs_tab->removerLinhas();

		if(rel)
		{
			if(rel->getRelationshipType()!=BaseRelationship::RELATIONSHIP_NN)
			{
				//Listando as colunas geradas pelo relacionamento
				cols=rel->getGeneratedColumns();
				count=cols.size();

				for(i=0; i < count; i++)
				{
					advanced_objs_tab->adicionarLinha();
					advanced_objs_tab->definirTextoCelula(QString::fromUtf8(cols[i]->getName()),i,0);
					advanced_objs_tab->definirTextoCelula(QString::fromUtf8(cols[i]->getTypeName()),i,1);
					advanced_objs_tab->definirDadoLinha(QVariant::fromValue<void *>(cols[i]), i);
				}

				//Listando as restrições geradas pelo relacionamento
				constrs=rel->getGeneratedConstraints();
				count=constrs.size();

				for(i=0, i1=advanced_objs_tab->obterNumLinhas(); i < count; i++,i1++)
				{
					advanced_objs_tab->adicionarLinha();
					advanced_objs_tab->definirTextoCelula(QString::fromUtf8(constrs[i]->getName()),i1,0);
					advanced_objs_tab->definirTextoCelula(QString::fromUtf8(constrs[i]->getTypeName()),i1,1);
					advanced_objs_tab->definirDadoLinha(QVariant::fromValue<void *>(constrs[i]), i1);
				}
			}
			else
			{
				//Lista a tabela gerada pelo relacionamento n-n
				tab=rel->getGeneratedTable();
				if(tab)
				{
					advanced_objs_tab->adicionarLinha();
					advanced_objs_tab->definirTextoCelula(QString::fromUtf8(tab->getName()),0,0);
					advanced_objs_tab->definirTextoCelula(QString::fromUtf8(tab->getTypeName()),0,1);
					advanced_objs_tab->definirDadoLinha(QVariant::fromValue<void *>(tab), 0);
				}
			}
		}
		else if(base_rel->getRelationshipType()==BaseRelationship::RELATIONSHIP_FK)
		{
			tab=dynamic_cast<Table *>(base_rel->getTable(BaseRelationship::DST_TABLE));
			dynamic_cast<Table *>(base_rel->getTable(BaseRelationship::SRC_TABLE))->getForeignKeys(constrs,false,tab);
			count=constrs.size();

			for(i=0, i1=advanced_objs_tab->obterNumLinhas(); i < count; i++, i1++)
			{
				advanced_objs_tab->adicionarLinha();
				advanced_objs_tab->definirTextoCelula(QString::fromUtf8(constrs[i]->getName()),i1,0);
				advanced_objs_tab->definirTextoCelula(QString::fromUtf8(constrs[i]->getTypeName()),i1,1);
				advanced_objs_tab->definirDadoLinha(QVariant::fromValue<void *>(constrs[i]), i1);
			}
		}

		advanced_objs_tab->limparSelecao();
		advanced_objs_tab->blockSignals(false);

	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void RelationshipWidget::showAdvancedObject(int idx)
{
	BaseObject *object=reinterpret_cast<BaseObject *>(advanced_objs_tab->obterDadoLinha(idx).value<void *>());
	bool prot=true;
	Table *tab=NULL;
	Constraint *constr=NULL;
	Column *col=NULL;

	switch(object->getObjectType())
	{
		case OBJ_COLUMN:
			col=dynamic_cast<Column *>(object);
			coluna_wgt->setAttributes(this->model, col->getParentTable(), this->op_list, col);
			coluna_wgt->show();
		break;

		case OBJ_CONSTRAINT:
			constr=dynamic_cast<Constraint *>(object);

			if(!constr->isAddedByRelationship())
			{
				prot=constr->isProtected();
				constr->setProtected(true);
			}

			restricao_wgt->setAttributes(this->model, constr->getParentTable(), this->op_list, constr);
			restricao_wgt->show();
			constr->setProtected(prot);
		break;

		default:
			//Not working with dynamic_cast ???
			tab=reinterpret_cast<Table *>(object);

			tab->setProtected(true);
			tabela_wgt->setAttributes(this->model, this->op_list, dynamic_cast<Schema *>(tab->getSchema()),
																tab,	tab->getPosition().x(), tab->getPosition().y());
			tabela_wgt->show();
			tab->setProtected(false);
		break;
	}
}

void RelationshipWidget::addObject(void)
{
	ObjectType obj_type=BASE_OBJECT;

	try
	{
		//Caso o objeto que acionou o método seja a tabela de atributos
		if(sender()==attributes_tab)
		{
			//Seleciona a tabela de atributos para manipulação
			obj_type=OBJ_COLUMN;
			tab=attributes_tab;
		}
		else
		{
			//Seleciona a tabela de restrições para manipulação
			obj_type=OBJ_CONSTRAINT;
			tab=constraints_tab;
		}

		//Caso o tipo do objeto seja uma coluna
		if(obj_type==OBJ_COLUMN)
		{
			//Exibe o formulário de criação de colunas (atributos)
			coluna_wgt->setAttributes(this->model, this->object, this->op_list, NULL);
			coluna_wgt->show();
		}
		else
		{
			//Exibe o formulário de criação de restrições
			restricao_wgt->setAttributes(this->model, this->object, this->op_list, NULL);
			restricao_wgt->show();
		}

		//Atualiza a lista de objetos exibindo o objeto recém criado
		listObjects(obj_type);
	}
	catch(Exception &e)
	{
		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void RelationshipWidget::editObject(int row)
{
	try
	{
		/* Anula a operação de encadeamento para que, em caso de erro na edição do objeto,
		 as demais operações encadeadas não sejam desfeitas desnecessariamente */
		op_list->ignoreOperationChain(true);

		//Caso seja a tabela de atributos que acionou o método
		if(sender()==attributes_tab)
		{
			/* Procede com a edição de uma coluna (atributo), sendo que a coluna a ser
			editada é obtida do dado armazenado na linha 'idx_lin' da tabela */
			coluna_wgt->setAttributes(this->model, this->object, this->op_list,
																reinterpret_cast<Column *>(attributes_tab->obterDadoLinha(row).value<void *>()));
			coluna_wgt->show();
		}
		else
		{
			//Edita uma restrição caso a tabela de restrições for a acionadora do método
			restricao_wgt->setAttributes(this->model, this->object, this->op_list,
																	 reinterpret_cast<Constraint *>(constraints_tab->obterDadoLinha(row).value<void *>()));
			restricao_wgt->show();
		}

		//Desfaz a anulação do encadeamento da lista de operações
		op_list->ignoreOperationChain(false);
	}
	catch(Exception &e)
	{
		op_list->ignoreOperationChain(false);
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void RelationshipWidget::showObjectData(TableObject *object, int row)
{
	TabelaObjetosWidget *tab=NULL;

	//Caso o tipo do objeto seja uma coluna
	if(object->getObjectType()==OBJ_COLUMN)
	{
		//Exibe o nome do tipo da coluna na tabela de atributos
		tab=attributes_tab;
		attributes_tab->definirTextoCelula(QString::fromUtf8(~dynamic_cast<Column *>(object)->getType()),row,1);
	}
	else
	{
		//Exibe o nome do tipo da restrição na tabela de restrições
		tab=constraints_tab;
		constraints_tab->definirTextoCelula(QString::fromUtf8(~dynamic_cast<Constraint *>(object)->getConstraintType()),row,1);
	}

	tab->definirTextoCelula(QString::fromUtf8(object->getName()),row,0);

	//Define como dado da linha o próprio objeto para facilitar referências ao mesmo
	tab->definirDadoLinha(QVariant::fromValue<void *>(object), row);
}

void RelationshipWidget::removeObjects(void)
{
	Relationship *rel=NULL;
	ObjectType obj_type=BASE_OBJECT;
	unsigned count, op_count=0, i;
	TableObject *object=NULL;

	try
	{
		//Obtém a referência ao relacionamento
		rel=dynamic_cast<Relationship *>(this->object);

		//Caso seja remoção de atributos
		if(sender()==attributes_tab)
		{
			//Obtém a quantidade de atributos do relacionamento
			obj_type=OBJ_COLUMN;
			count=rel->getAttributeCount();
		}
		else
		{
			//Obtém a quantidade de restrições do relacionamento
			obj_type=OBJ_CONSTRAINT;
			count=rel->getConstraintCount();
		}

		/* Armazena a quantidade de operações antes da remoção de objetos.
		 Caso um erro seja gerado e a quantidade de operações na lista
		 seja diferente do valor na variável 'qtd_op' indica que operações
		 foram inseridas na lista e precisam ser removidas */
		op_count=op_list->getCurrentSize();

		for(i=0; i < count; i++)
		{
			//Obtém o objeto do relacionamento
			object=rel->getObject(0, obj_type);

			//Tenta removê-lo do relacionamento
			rel->removeObject(object);

			//Adiciona o objeto removido na lista de operações para ser restaurado se necessário
			op_list->registerObject(object, Operation::OBJECT_REMOVED, 0, rel);
		}
	}
	catch(Exception &e)
	{
		/* Caso a quantidade de operações seja diferente da quantidade inicial
		 obtida antes da remoção dos objetos */
		if(op_count < op_list->getCurrentSize())
		{
			//Obtém a quantidade de operações que necessitam ser removidas
			count=op_list->getCurrentSize()-op_count;

			/* Anula o encadeamento de operações para que as mesmas seja
			desfeitas uma a uma ignorando o encadeamento */
			op_list->ignoreOperationChain(true);

			/* Desfaz as operações na quantidade calculada e remove a
			operação da lista */
			for(i=0; i < count; i++)
			{
				op_list->undoOperation();
				op_list->removeLastOperation();
			}

			//Desfaz a anulação do encadeamento
			op_list->ignoreOperationChain(false);
		}

		//Atualiza a lista de objeto do relacionamento
		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void RelationshipWidget::removeObject(int row)
{
	Relationship *rel=NULL;
	ObjectType obj_type=BASE_OBJECT;
	TableObject *object=NULL;

	try
	{
		//Obtém a referência ao relacionamento
		rel=dynamic_cast<Relationship *>(this->object);

		//Caso o sender do método seja a tabela de atributos
		if(sender()==attributes_tab)
			//Marca que o tipo do objeto a ser removido é uma coluna
			obj_type=OBJ_COLUMN;
		else
			obj_type=OBJ_CONSTRAINT;

		//Obtém o objeto no índice especificado
		object=rel->getObject(row, obj_type);

		//Remove o objeto e o adiciona a lista de operações para ser restaurado se necessário
		rel->removeObject(object);

		op_list->registerObject(object, Operation::OBJECT_REMOVED, 0, rel);
	}
	catch(Exception &e)
	{
		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void RelationshipWidget::applyConfiguration(void)
{
	try
	{
		Relationship *rel=NULL;
		unsigned rel_type, count, i;
		vector<unsigned> col_ids;

		/* Devido a complexidade da classe Relacionamento e a forte ligação entre todos os
		 relacinamentos do modelo, é necessário armazenar o XML dos objetos especiais e
		 desconectar TODOS os relacionamentos, executar a modificação no
		 relacionamento e logo após revalidar todos os demais */
		if(this->object->getObjectType()==OBJ_RELATIONSHIP)
		{
			model->storeSpecialObjectsXML();
			model->disconnectRelationships();
		}

		if(!this->new_object && this->object->getObjectType()==OBJ_RELATIONSHIP)
		{
			//Adiciona o relacionamento   lista de operações antes de ser modificado
			op_list->registerObject(this->object, Operation::OBJECT_MODIFIED);
		}

		//Aplica as configurações básicas
		BaseObjectWidget::applyConfiguration();

		//Caso o objeto seja um relacionamento tabela-tabela
		if(this->object->getObjectType()==OBJ_RELATIONSHIP)
		{
			//Obtém a referência ao mesmo fazendo o cast correto
			rel=dynamic_cast<Relationship *>(this->object);
			rel_type=rel->getRelationshipType();
			rel->blockSignals(true);

			/* Atribui os valores configurados no formulário ao relacionamento.
			Alguns campos são atribuído ao objeto somente para um tipo específico
			de relacionamento */
			rel->setTableSuffix(BaseRelationship::SRC_TABLE, src_suffix_edt->text());
			rel->setTableSuffix(BaseRelationship::DST_TABLE, dst_suffix_edt->text());
			rel->setAutomaticSuffix(auto_suffix_chk->isChecked());

			rel->setMandatoryTable(BaseRelationship::SRC_TABLE, false);
			rel->setMandatoryTable(BaseRelationship::DST_TABLE, false);

			if(table1_mand_chk->isEnabled())
				rel->setMandatoryTable(BaseRelationship::SRC_TABLE, table1_mand_chk->isChecked());

			if(table2_mand_chk->isEnabled())
				rel->setMandatoryTable(BaseRelationship::DST_TABLE, table2_mand_chk->isChecked());

			if(rel_type==BaseRelationship::RELATIONSHIP_GEN ||
				 rel_type==BaseRelationship::RELATIONSHIP_DEP)
			{
				//Obtém os ids das colunas selecionadas como participantes da chave primária especial
				count=rel_columns_lst->count();
				for(i=0; i < count; i++)
				{
					//Caso o item na lista esteja selecionado seu id é armazenado no vetor de ids
					if(rel_columns_lst->item(i)->checkState()==Qt::Checked)
						col_ids.push_back(i);
				}

				//Atribui o vetor de ids configurado acima como sendo os ids das colunas da chave primária especial
				rel->setSpecialPrimaryKeyCols(col_ids);
			}
			//Campos específicos para relacionamentos 1-n e 1-1
			else if(rel_type==BaseRelationship::RELATIONSHIP_1N ||
							rel_type==BaseRelationship::RELATIONSHIP_11)
			{
				rel->setIdentifier(identifier_chk->isChecked());
				rel->setDeferrable(deferrable_chk->isChecked());
				rel->setDeferralType(DeferralType(deferral_cmb->currentText()));
			}
			//Campos específicos para relacionamentos n-n
			else if(rel_type==BaseRelationship::RELATIONSHIP_NN)
				rel->setTableNameRelNN(relnn_tab_name_edt->text());

			try
			{
				/* Caso o relacinamento seja de dependência, generalização ou
			 identificador verifica se existe redundância de relacionamentos */
				if(rel_type==BaseRelationship::RELATIONSHIP_DEP ||
					 rel_type==BaseRelationship::RELATIONSHIP_GEN ||
					 rel->isIdentifier())
					model->checkRelationshipRedundancy(rel);

				if(rel_type!=BaseRelationship::RELATIONSHIP_FK)
					/* Faz a validação dos relacionamentos para refletir a nova configuração
				do relacionamento */
					model->validateRelationships();

				rel->blockSignals(false);
				rel->setModified(true);
			}
			catch(Exception &e)
			{
				/* O único erro que é desconsiderado é o de invalidação de objetos, pois,
			 mesmo com a restauração do estado original do relacionamento estes
			 objetos não são recuperados */
				if(e.getErrorType()==ERR_INVALIDATED_OBJECTS)
					//Exibe uma mensagem de erro com o conteúdo da exceção
					caixa_msg->show(e);
				//Para os demais erros a exceção é encaminhada
				else
					throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
			}
		}

		//Finaliza o encademanto de operações aberto
		op_list->finishOperationChain();

		//Finaliza a configuração do relacionamento
		finishConfiguration();
	}
	catch(Exception &e)
	{
		/* Cancela a configuração o objeto removendo a ultima operação adicionada
		 referente ao objeto editado/criado e desaloca o objeto
		 caso o mesmo seja novo */
		op_list->ignoreOperationChain(true);
		this->cancelConfiguration();
		op_list->ignoreOperationChain(false);

		/* Faz a validação dos relacionamentos para refletir a nova configuração
		 do relacionamento */
		model->validateRelationships();

		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void RelationshipWidget::cancelConfiguration(void)
{
	if(op_list->isOperationChainStarted())
		op_list->finishOperationChain();

	//Caso a lista de operações sofreu modificações
	if(operation_count < op_list->getCurrentSize())
		/* Executa o cancelamento da configuração e remove as operações
		 adicionadas durante a edição do relacionamento */
		BaseObjectWidget::cancelConfiguration();
}
