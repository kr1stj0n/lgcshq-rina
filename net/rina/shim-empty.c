/*
 *  Empty Shim IPC Process (Shim template that should be used as a reference)
 *
 *    Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>

#include <linux/slab.h>
#include <linux/list.h>

#define RINA_PREFIX "shim-empty"

#include "logs.h"
#include "common.h"
#include "utils.h"
#include "kipcm.h"
#include "shim.h"

/* This structure will contains per-instance data */
struct shim_instance_data {
        ipc_process_id_t id;
};

/*
 * NOTE:
 *   The functions that have to be exported for shim-instance. The caller
 *   ensures the parameters are not empty, therefore only assertions should be
 *   added
 */

static int empty_flow_allocate_request(struct shim_instance_data * data,
                                       const struct name_t *       source,
                                       const struct name_t *       dest,
                                       const struct flow_spec_t *  flow_spec,
                                       port_id_t *                 id)
{
        LOG_FBEGN;

        ASSERT(data);
        ASSERT(source);
        ASSERT(dest)

        LOG_FEXIT;

        return 0;
}

static int empty_flow_allocate_response(struct shim_instance_data * data,
                                        port_id_t                   id,
                                        response_reason_t *         response)
{
        LOG_FBEGN;

        ASSERT(data);
        ASSERT(response);

        LOG_FEXIT;

        return 0;
}

static int empty_flow_deallocate(struct shim_instance_data * data,
                                 port_id_t                   id)
{
        LOG_FBEGN;

        ASSERT(data);

        LOG_FEXIT;

        return 0;
}

static int empty_application_register(struct shim_instance_data * data,
                                      const struct name_t *       name)
{
        LOG_FBEGN;

        ASSERT(data);
        ASSERT(name);

        LOG_FEXIT;

        return 0;
}

static int empty_application_unregister(struct shim_instance_data * data,
                                        const struct name_t *       name)
{
        LOG_FBEGN;

        ASSERT(data);
        ASSERT(name);

        LOG_FEXIT;

        return 0;
}

static int empty_sdu_write(struct shim_instance_data * data,
                           port_id_t                   id,
                           const struct sdu_t *        sdu)
{
        LOG_FBEGN;

        ASSERT(data);
        ASSERT(sdu);

        LOG_FEXIT;

        return 0;
}

static int empty_sdu_read(struct shim_instance_data * data,
                          port_id_t                   id,
                          struct sdu_t *              sdu)
{
        LOG_FBEGN;

        ASSERT(data);
        ASSERT(sdu);

        LOG_FEXIT;

        return 0;
}

/*
 * The shim_instance ops are common to all the shim instances therefore
 * there's no real need to take a dynamically allocated buffer. Let's use a
 * static struct containing all the pointers and share it among all the
 * instances.
 */
static struct shim_instance_ops empty_instance_ops = {
        .flow_allocate_request  = empty_flow_allocate_request,
        .flow_allocate_response = empty_flow_allocate_response,
        .flow_deallocate        = empty_flow_deallocate,
        .application_register   = empty_application_register,
        .application_unregister = empty_application_unregister,
        .sdu_write              = empty_sdu_write,
        .sdu_read               = empty_sdu_read,
};

/*
 * This structure (static again, there's no need to dynamically allocate a
 * buffer) will hold all the shim data
 */
static struct shim_data {
        struct list_head * instances;
} empty_data;

/*
 * We maintain a shim pointer since the module is loaded once (and we need it
 * during module unloading)
*/
static struct shim * empty_shim = NULL;

/*
 * NOTE:
 *   The functions that have to be exported to the shim-layer.
 */

/* Initializes the shim_data structure */
static int empty_init(struct shim_data * data)
{
        LOG_FBEGN;

        ASSERT(data);

        bzero(&empty_data, sizeof(empty_data));

        INIT_LIST_HEAD(data->instances);

        LOG_FEXIT;

        return 0;
}

/*
 * Finalizes (destroys) the shim_data structure, releasing resources allocated
 * by empty_init
 */
static int empty_fini(struct shim_data * data)
{
        LOG_FBEGN;

        ASSERT(data);

        /*
         * NOTE:
         *   All the instances should be removed by the shims layer, no work is
         *   needed here. In theory, a check for empty list should be added
         *   here
         */

        LOG_FEXIT;

        return 0;
}

static struct shim_instance * empty_create(struct shim_data * data,
                                           ipc_process_id_t   id)
{
        struct shim_instance * inst;

        LOG_FBEGN;

        ASSERT(data);

        /* Create and instance */
        inst = kzalloc(sizeof(*inst), GFP_KERNEL);
        if (!inst) {
                LOG_ERR("Cannot allocate %zd bytes of memory", sizeof(*inst));
                return NULL;
        }

        /* fill it properly */
        inst->ops  = &empty_instance_ops;
        inst->data = kzalloc(sizeof(struct shim_instance_data), GFP_KERNEL);
        if (!inst->data) {
                LOG_ERR("Cannot allocate %zd bytes of memory",
                        sizeof(*inst->data));
                kfree(inst);
                return NULL;
        }

        inst->data->id = id;

        /*
         * Bind the shim-instance to the shims set, to keep all our data
         * structures linked (somewhat) together
         */
        list_add(data->instances, inst->data);

        LOG_FEXIT;

        return inst;
}

static struct shim_instance * empty_configure(struct shim_data *         data,
                                              struct shim_instance *     inst,
                                              const struct shim_config * cfg)
{
        LOG_FBEGN;

        ASSERT(data);
        ASSERT(inst);
        ASSERT(cfg);

        /* Do we have that instance ? */

        /* Yes */

        /* Use configuration values on that instance */

        /*
         * Instance might change (reallocation), return the updated pointer
         * if needed. We don't re-allocate our instance so we'll be returning
         * the same pointer.
         */

        LOG_FEXIT;

        return inst;
}

static int empty_destroy(struct shim_data *       data,
                         struct shim_instance *   instance)
{
        LOG_FBEGN;

        ASSERT(data);
        ASSERT(instance);

        /* Retrieve the instance */

        /* Destroy it */

        /* Unbind from the instances set */

        LOG_FEXIT;

        return 0;
}

static struct shim_ops empty_ops = {
        .init      = empty_init,
        .fini      = empty_fini,
        .create    = empty_create,
        .destroy   = empty_destroy,
        .configure = empty_configure,
};

/* FIXME: To be removed ABSOLUTELY */
extern struct kipcm * default_kipcm;

static int __init mod_init(void)
{
        LOG_FBEGN;

        /*
         * Pass data and ops to the upper layer.
         *
         * If the init function is not empty (our case, points to empty_fini)
         * it will get called and empty_data will be initialized.
         *
         * If the init function is not present (empty_ops.init == NULL)
         *
         */

        empty_shim = kipcm_shim_register(default_kipcm,
                                         "shim-empty",
                                         &empty_data,
                                         &empty_ops);
        if (!empty_shim) {
                LOG_CRIT("Initialization failed");

                LOG_FEXIT;
                return -1;
        }

        LOG_FEXIT;

        return 0;
}

static void __exit mod_exit(void)
{
        LOG_FBEGN;

        /*
         * Upon unregistration empty_ops.fini will be called (if present), that
         * function will be in charge to release all the resources allocated
         * during the shim lifetime.
         *
         * The upper layers will call empty_ops.create and empty_ops.destroy so
         * empty_ops.fini should release the resources allocated by
         * empty_ops.init
         */
        if (kipcm_shim_unregister(default_kipcm, empty_shim)) {
                LOG_CRIT("Cannot unregister");
                return;
        }

        LOG_FEXIT;
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_DESCRIPTION("RINA Empty Shim IPC");

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Francesco Salvestrini <f.salvestrini@nextworks.it>");
