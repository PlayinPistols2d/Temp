WITH RECURSIVE task_hierarchy AS (
    -- Anchor member: Select all root tasks by post_id
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

    -- Recursive member: Select all child tasks recursively
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

-- Final select to get all tasks in the desired hierarchical order
SELECT * 
FROM task_hierarchy
ORDER BY 
    root_id ASC,        -- Ensure all tasks under the same root are grouped together
    depth ASC,          -- Ensure the root task comes before its children
    parent_operation_id ASC NULLS FIRST,  -- Group children under their parent, root tasks first
    priority ASC;       -- Order by priority within each level