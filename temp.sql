WITH RECURSIVE task_hierarchy AS (
    -- Первый шаг: выбираем все корневые задачи с parent_operation_id = NULL
    SELECT 
        id,
        name,
        type,
        parent_operation_id,
        priority,
        post_id,
        id AS root_id,
        0 AS depth
    FROM operations
    WHERE parent_operation_id IS NULL AND post_id = :post_id

    UNION ALL

    -- Рекурсивный шаг: выбираем все дочерние задачи, связывая их с родительскими
    SELECT 
        o.id,
        o.name,
        o.type,
        o.parent_operation_id,
        o.priority,
        o.post_id,
        th.root_id,
        th.depth + 1 AS depth
    FROM operations o
    INNER JOIN task_hierarchy th ON o.parent_operation_id = th.id
    WHERE o.post_id = :post_id
)

-- Окончательный запрос для вывода задач в требуемом порядке
SELECT 
    id, 
    name, 
    type, 
    parent_operation_id, 
    priority, 
    post_id
FROM task_hierarchy
ORDER BY 
    root_id ASC,      -- Группируем задачи по корневым задачам
    depth ASC,        -- Сначала выводим родительские задачи, затем дочерние
    priority ASC;     -- Упорядочиваем задачи по приоритету