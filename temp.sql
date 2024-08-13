CREATE OR REPLACE FUNCTION manage_operation_priority_and_post_id()
RETURNS TRIGGER AS $$
DECLARE
    max_priority INT;
BEGIN
    IF TG_OP = 'INSERT' THEN
        -- Handle Insert operation
        IF NEW.parent_operation_id IS NULL THEN
            -- Find max priority among root operations
            SELECT COALESCE(MAX(priority), 0) INTO max_priority
            FROM operations
            WHERE parent_operation_id IS NULL;
            
            -- Set the new operation's priority
            NEW.priority = max_priority + 1;
        ELSE
            -- Find max priority among sibling operations
            SELECT COALESCE(MAX(priority), 0) INTO max_priority
            FROM operations
            WHERE parent_operation_id = NEW.parent_operation_id;
            
            -- Set the new operation's priority
            NEW.priority = max_priority + 1;

            -- Inherit post_id from the parent
            SELECT post_id INTO NEW.post_id
            FROM operations
            WHERE id = NEW.parent_operation_id;
        END IF;
        RETURN NEW;

    ELSIF TG_OP = 'DELETE' THEN
        -- Handle Delete operation
        IF OLD.parent_operation_id IS NULL THEN
            -- Adjust priorities of other root operations
            UPDATE operations
            SET priority = priority - 1
            WHERE parent_operation_id IS NULL
            AND priority > OLD.priority;
        ELSE
            -- Adjust priorities of sibling operations
            UPDATE operations
            SET priority = priority - 1
            WHERE parent_operation_id = OLD.parent_operation_id
            AND priority > OLD.priority;
        END IF;
        RETURN OLD;

    ELSIF TG_OP = 'UPDATE' THEN
        -- Handle Update operation
        IF OLD.priority <> NEW.priority THEN
            -- Adjust sibling priorities to accommodate the new priority
            IF NEW.parent_operation_id IS NULL THEN
                -- Adjust root operations
                UPDATE operations
                SET priority = CASE
                    WHEN priority >= NEW.priority THEN priority + 1
                    ELSE priority
                END
                WHERE parent_operation_id IS NULL
                AND id <> NEW.id;
            ELSE
                -- Adjust sibling operations
                UPDATE operations
                SET priority = CASE
                    WHEN priority >= NEW.priority THEN priority + 1
                    ELSE priority
                END
                WHERE parent_operation_id = NEW.parent_operation_id
                AND id <> NEW.id;
            END IF;
        END IF;

        -- If post_id of parent changes, propagate to children
        IF OLD.post_id <> NEW.post_id THEN
            UPDATE operations
            SET post_id = NEW.post_id
            WHERE parent_operation_id = NEW.id;
        END IF;

        RETURN NEW;
    END IF;
END;
$$ LANGUAGE plpgsql;