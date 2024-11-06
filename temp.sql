CREATE OR REPLACE FUNCTION manage_task_priority()
RETURNS TRIGGER AS $$
DECLARE
    max_priority INT;
BEGIN
    IF TG_OP = 'INSERT' THEN
        -- Handle Insert operation
        IF NEW.parent_operation_id IS NULL THEN
            -- Find max priority among root tasks
            SELECT COALESCE(MAX(priority), 0) INTO max_priority
            FROM tasks
            WHERE parent_operation_id IS NULL;

            -- Set the new task's priority
            NEW.priority = max_priority + 1;
        ELSE
            -- Find max priority among sibling tasks
            SELECT COALESCE(MAX(priority), 0) INTO max_priority
            FROM tasks
            WHERE parent_operation_id = NEW.parent_operation_id;

            -- Set the new task's priority
            NEW.priority = max_priority + 1;
        END IF;
        RETURN NEW;

    ELSIF TG_OP = 'DELETE' THEN
        -- Handle Delete operation
        IF OLD.parent_operation_id IS NULL THEN
            -- Adjust priorities of other root tasks
            UPDATE tasks
            SET priority = priority - 1
            WHERE parent_operation_id IS NULL
            AND priority > OLD.priority;

            -- Reorder priorities to remove gaps
            UPDATE tasks
            SET priority = sub.new_priority
            FROM (
                SELECT id, ROW_NUMBER() OVER (ORDER BY priority) AS new_priority
                FROM tasks
                WHERE parent_operation_id IS NULL
            ) AS sub
            WHERE tasks.id = sub.id;
        ELSE
            -- Adjust priorities of sibling tasks
            UPDATE tasks
            SET priority = priority - 1
            WHERE parent_operation_id = OLD.parent_operation_id
            AND priority > OLD.priority;

            -- Reorder priorities to remove gaps
            UPDATE tasks
            SET priority = sub.new_priority
            FROM (
                SELECT id, ROW_NUMBER() OVER (ORDER BY priority) AS new_priority
                FROM tasks
                WHERE parent_operation_id = OLD.parent_operation_id
            ) AS sub
            WHERE tasks.id = sub.id;
        END IF;

        RETURN OLD;

    ELSIF TG_OP = 'UPDATE' THEN
        -- Handle Update operation: allowing manual change of priority
        IF OLD.priority <> NEW.priority THEN
            -- Shift priorities to accommodate the new priority
            IF NEW.parent_operation_id IS NULL THEN
                -- Adjust root tasks
                UPDATE tasks
                SET priority = CASE
                    WHEN priority >= NEW.priority THEN priority + 1
                    ELSE priority
                END
                WHERE parent_operation_id IS NULL
                AND id <> NEW.id;
            ELSE
                -- Adjust sibling tasks
                UPDATE tasks
                SET priority = CASE
                    WHEN priority >= NEW.priority THEN priority + 1
                    ELSE priority
                END
                WHERE parent_operation_id = NEW.parent_operation_id
                AND id <> NEW.id;
            END IF;
        END IF;
        RETURN NEW;
    END IF;
END;
$$ LANGUAGE plpgsql;