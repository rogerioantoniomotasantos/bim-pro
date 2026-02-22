.PHONY: help build up down logs clean

help: ## Mostrar ajuda
	@echo "BIM Professional Platform - Comandos disponíveis:"
	@echo ""
	@echo "  make build    - Build de todas as imagens Docker"
	@echo "  make up       - Iniciar todos os serviços"
	@echo "  make down     - Parar todos os serviços"
	@echo "  make logs     - Ver logs de todos os serviços"
	@echo "  make clean    - Limpar tudo (volumes incluídos)"
	@echo ""

build: ## Build das imagens
	docker-compose build

up: ## Iniciar serviços
	docker-compose up -d
	@echo "✅ Serviços iniciados!"
	@echo "   Viewer: http://localhost"
	@echo "   API: http://localhost:8000"

down: ## Parar serviços
	docker-compose down

logs: ## Ver logs
	docker-compose logs -f

clean: ## Limpar tudo
	docker-compose down -v
	docker system prune -f

status: ## Ver status
	docker-compose ps

restart: down up ## Reiniciar
