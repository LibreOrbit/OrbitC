/*
 * ================================================================================
 * File: modules/containers/src/orbitc_ringbuf.c
 * ================================================================================
 */
#include "orbitc_ringbuf.h"

#if (ENABLE_CONTAINER_RINGBUF == 1)

#define ORBITC_RINGBUF_MAGIC ((ui32_t)0x52425546UL) /* "RBUF" */

status_t orbitc_ringbuf_init(
    ringbuf_t *rb,
    void *buf,
    usize_t capacity,
    ringbuf_mode_t mode)
{
    status_t st = STATUS_OK;
    bool_t   parameters_valid = ORBITC_FALSE;

    orbitc_assert_not_null(
        rb,
        &st,
        STATUS_MOD_CONTAINERS,
        (ui8_t)CONTAINERS_CODE_NULL_PTR);

    orbitc_assert_not_null(
        buf,
        &st,
        STATUS_MOD_CONTAINERS,
        (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        parameters_valid =
            ((capacity > 0U) &&
             ((mode == RINGBUF_MODE_REJECT) ||
              (mode == RINGBUF_MODE_OVERWRITE)))
                ? ORBITC_TRUE
                : ORBITC_FALSE;

        orbitc_assert_param(
            parameters_valid,
            &st,
            STATUS_SEV_ERROR,
            STATUS_MOD_CONTAINERS,
            (ui8_t)CONTAINERS_CODE_INVALID_PARAM);
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        rb->buf        = (byte_t *)buf;
        rb->capacity   = capacity;
        rb->head       = 0U;
        rb->tail       = 0U;
        rb->count      = 0U;
        rb->high_water = 0U;
        rb->mode       = mode;
        rb->init_magic = ORBITC_RINGBUF_MAGIC;
    }

    return st;
}

status_t orbitc_ringbuf_reset(ringbuf_t *rb)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(rb, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->init_magic != ORBITC_RINGBUF_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        rb->head  = 0U;
        rb->tail  = 0U;
        rb->count = 0U;
    }

    return st;
}

status_t orbitc_ringbuf_push_byte(ringbuf_t *rb, byte_t val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(rb, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->init_magic != ORBITC_RINGBUF_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->count == rb->capacity)
        {
            if (rb->mode == RINGBUF_MODE_REJECT)
            {
                orbitc_status_latch(
                    &st,
                    STATUS_MAKE(
                        STATUS_SEV_ERROR,
                        STATUS_MOD_CONTAINERS,
                        STATUS_CLS_CAPACITY,
                        (ui8_t)CONTAINERS_CODE_FULL));
            }
            else
            {
                /* Overwrite mode: advance tail to drop the oldest byte */
                rb->tail = (rb->tail + 1U) % rb->capacity;
            }
        }
        else
        {
            rb->count += 1U;
            if (rb->count > rb->high_water)
            {
                rb->high_water = rb->count;
            }
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        rb->buf[rb->head] = val;
        rb->head          = (rb->head + 1U) % rb->capacity;
    }

    return st;
}

status_t orbitc_ringbuf_pop_byte(ringbuf_t *rb, byte_t *out_val)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(rb, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_val, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->init_magic != ORBITC_RINGBUF_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->count == 0U)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_NOT_FOUND,
                    (ui8_t)CONTAINERS_CODE_EMPTY));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_val  = rb->buf[rb->tail];
        rb->tail  = (rb->tail + 1U) % rb->capacity;
        rb->count -= 1U;
    }

    return st;
}

status_t orbitc_ringbuf_peek_byte(const ringbuf_t *rb, usize_t offset, byte_t *out_val)
{
    status_t st = STATUS_OK;
    usize_t  idx;

    orbitc_assert_not_null(rb, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_val, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->init_magic != ORBITC_RINGBUF_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (offset >= rb->count)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_OUT_OF_RANGE,
                    (ui8_t)CONTAINERS_CODE_OUT_OF_RANGE));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        idx      = (rb->tail + offset) % rb->capacity;
        *out_val = rb->buf[idx];
    }

    return st;
}

status_t orbitc_ringbuf_write(ringbuf_t *rb, const void *data, usize_t len)
{
    status_t      st = STATUS_OK;
    const byte_t *src;
    usize_t       i;

    orbitc_assert_not_null(rb, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(data, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->init_magic != ORBITC_RINGBUF_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (len > 0U)
        {
            if ((rb->mode == RINGBUF_MODE_REJECT) && (len > (rb->capacity - rb->count)))
            {
                orbitc_status_latch(
                    &st,
                    STATUS_MAKE(
                        STATUS_SEV_ERROR,
                        STATUS_MOD_CONTAINERS,
                        STATUS_CLS_CAPACITY,
                        (ui8_t)CONTAINERS_CODE_FULL));
            }
            else if ((rb->mode == RINGBUF_MODE_OVERWRITE) && (len > rb->capacity))
            {
                orbitc_status_latch(
                    &st,
                    STATUS_MAKE(
                        STATUS_SEV_ERROR,
                        STATUS_MOD_CONTAINERS,
                        STATUS_CLS_CAPACITY,
                        (ui8_t)CONTAINERS_CODE_FULL));
            }
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (len > 0U)
        {
            src = (const byte_t *)data;
            for (i = 0U; i < len; ++i)
            {
                orbitc_status_latch(&st, orbitc_ringbuf_push_byte(rb, src[i]));
                if (orbitc_status_is_ok(st) == ORBITC_FALSE)
                {
                    break;
                }
            }
        }
    }

    return st;
}

status_t orbitc_ringbuf_read(ringbuf_t *rb, void *out_data, usize_t len)
{
    status_t st = STATUS_OK;
    byte_t  *dst;
    usize_t  i;

    orbitc_assert_not_null(rb, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_data, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->init_magic != ORBITC_RINGBUF_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (len > 0U)
        {
            if (len > rb->count)
            {
                orbitc_status_latch(
                    &st,
                    STATUS_MAKE(
                        STATUS_SEV_ERROR,
                        STATUS_MOD_CONTAINERS,
                        STATUS_CLS_NOT_FOUND,
                        (ui8_t)CONTAINERS_CODE_EMPTY));
            }
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (len > 0U)
        {
            dst = (byte_t *)out_data;
            for (i = 0U; i < len; ++i)
            {
                orbitc_status_latch(&st, orbitc_ringbuf_pop_byte(rb, &dst[i]));
                if (orbitc_status_is_ok(st) == ORBITC_FALSE)
                {
                    break;
                }
            }
        }
    }

    return st;
}

status_t orbitc_ringbuf_get_count(const ringbuf_t *rb, usize_t *out_count)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(rb, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_count, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->init_magic != ORBITC_RINGBUF_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_count = rb->count;
    }

    return st;
}

status_t orbitc_ringbuf_get_free(const ringbuf_t *rb, usize_t *out_free)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(rb, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_free, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        if (rb->init_magic != ORBITC_RINGBUF_MAGIC)
        {
            orbitc_status_latch(
                &st,
                STATUS_MAKE(
                    STATUS_SEV_ERROR,
                    STATUS_MOD_CONTAINERS,
                    STATUS_CLS_INVALID_PARAM,
                    (ui8_t)CONTAINERS_CODE_NOT_INIT));
        }
    }

    if (orbitc_status_is_ok(st) == ORBITC_TRUE)
    {
        *out_free = rb->capacity - rb->count;
    }

    return st;
}

#else
/* Prevent empty translation unit warning under strict ISO C99 */
typedef int orbitc_ringbuf_disabled_tu_t;
#endif /* ENABLE_CONTAINER_RINGBUF == 1 */